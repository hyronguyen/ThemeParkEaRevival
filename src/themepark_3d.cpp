// Native original-model inspection workbench, not gameplay emulation.
#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <GL/gl.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
namespace fs=std::filesystem;
using Mat=std::array<float,16>;
struct Vertex{float x,y,z,u,v;};
struct Node{int parent;std::array<float,10> frame;Mat world;bool locator=false;};
struct Part{int node,mesh;float alpha;std::string texture;std::vector<Vertex> vertices;};
struct Model{int id=0;std::vector<Node> nodes;std::vector<Part> parts;std::array<float,3> center{};float radius=1;size_t triangles=0;};
struct Entry{int id;fs::path path;size_t triangles;};
struct Track{int frames;std::array<std::vector<float>,10> channels;};
struct Animation{int group=-1,frameMS=33;std::vector<Track> tracks;};
struct SceneInstance{char kind;int row,model;float x,y,z,angle;std::string texture;float cullRadius=0;};
struct SceneMesh{Model model;GLuint list=0;};
static fs::path root;
static std::vector<Entry> catalog;
static std::map<int,std::map<int,int>> locatorCatalog;
static Model model;
static Animation animation;
static std::map<int,std::pair<int,fs::path>> animationCatalog;
static bool playing=true,bindPose=false;
static double animationMS=0;
static double waterFXTime=0;
static ULONGLONG lastTick=0;
static int selected=0,width=1280,height=800;
static float yaw=-35,pitch=62,zoom=1,panX=0,panY=0;
static bool wire=false,grid=true,drag=false,pan=false,testing=false;
static bool game=false;
static bool startupActive=false,startupReady=false;
static float startupProgress=.05f;
static std::string startupMessage="Preparing your park";
static void startupRender(bool present);
static POINT mouse{};
static HWND window;
static HDC dc;
static HGLRC context;
static bool multisampling=false;
static GLuint fontBase=0;
static std::map<std::string,GLuint> textures;
static std::vector<SceneInstance> scene;
static std::map<std::pair<int,std::string>,SceneMesh> sceneMeshes;
static bool park=false,terrain=true,walls=true,paths=true;
static std::array<float,3> sceneCenter{};
static float sceneRadius=1;
static Mat identity(){return {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};}
static Mat multiply(const Mat&a,const Mat&b){Mat r{};for(int c=0;c<4;c++)for(int i=0;i<4;i++)for(int k=0;k<4;k++)r[c*4+i]+=a[k*4+i]*b[c*4+k];return r;}
static Mat transform(const std::array<float,10>&t){
    float x=t[3],y=t[4],z=t[5],w=t[6];
    Mat m={1-2*y*y-2*z*z,2*x*y+2*z*w,2*x*z-2*y*w,0,
           2*x*y-2*z*w,1-2*x*x-2*z*z,2*y*z+2*x*w,0,
           2*x*z+2*y*w,2*y*z-2*x*w,1-2*x*x-2*y*y,0,t[7],t[8],t[9],1};
    for(int c=0;c<3;c++)for(int r=0;r<3;r++)m[c*4+r]*=t[c];return m;
}
static std::array<float,3> point(const Mat&m,const Vertex&v){return {m[0]*v.x+m[4]*v.y+m[8]*v.z+m[12],m[1]*v.x+m[5]*v.y+m[9]*v.z+m[13],m[2]*v.x+m[6]*v.y+m[10]*v.z+m[14]};}
static void require(bool ok,const char*why){if(!ok)throw std::runtime_error(why);}
static fs::path assetPath(const std::string&s){
    fs::path p=fs::u8path(s);require(!p.is_absolute()&&!p.has_root_name()&&s.find(':')==std::string::npos,"Absolute asset path");
    for(const auto&c:p)require(c!=L"..","Asset path escapes extraction");return root/L"extracted"/p;
}
static void loadCatalog(){
    std::ifstream in(root/L"extracted/runtime/models.tsv");require(bool(in),"Run tools/extract_models.py first");std::string row;
    while(std::getline(in,row)){if(row.empty()||row[0]=='#')continue;Entry e;std::string path;std::istringstream s(row);
        require(bool(s>>e.id>>path>>e.triangles),"Invalid model catalog");e.path=assetPath(path);catalog.push_back(e);require(catalog.size()<=10000,"Model catalog too large");}
    require(!catalog.empty(),"Empty model catalog");
    std::ifstream locators(root/L"extracted/runtime/model_locators.tsv");require(bool(locators),"Run tools/export_model_locators.py first");int lm,ln,li;while(locators>>lm>>ln>>li){require(lm>=0&&lm<int(catalog.size())&&ln>=0&&li>=1&&li<=34,"Invalid locator catalog");locatorCatalog[lm][ln]=li;}
    std::ifstream anim(root/L"extracted/runtime/animations.tsv");
    while(std::getline(anim,row)){if(row.empty()||row[0]=='#')continue;int mi,gi;std::string path;std::istringstream s(row);require(bool(s>>mi>>gi>>path),"Invalid animation catalog");animationCatalog[mi]={gi,assetPath(path)};}
}
static Animation loadAnimation(int modelID,size_t nodes){
    Animation a;auto it=animationCatalog.find(modelID);if(it==animationCatalog.end())return a;
    std::ifstream in(it->second.second);std::string magic;size_t nn;
    require(bool(in>>magic>>a.group>>nn>>a.frameMS)&&magic=="TPA1"&&nn==nodes&&a.group==it->second.first&&a.frameMS==33,"Invalid animation header");
    for(size_t n=0;n<nn;n++){Track t;char tag;int id;require(bool(in>>tag>>id>>t.frames)&&tag=='T'&&t.frames>0&&t.frames<=100000,"Invalid animation track");
        for(auto&channel:t.channels){size_t count;require(bool(in>>count)&&(count==1||count==size_t(t.frames)),"Invalid animation channel length");channel.resize(count);for(float&v:channel)require(bool(in>>v)&&std::isfinite(v)&&std::abs(v)<1e8,"Invalid animation sample");}a.tracks.push_back(std::move(t));}
    require(!(in>>magic),"Trailing animation data");return a;
}
static std::array<float,10> sample(const Track&t,double ms,int frameMS){
    double frame=std::fmod(ms/frameMS,double(t.frames));int lo=int(std::floor(frame)),hi=int(std::ceil(frame))%t.frames;float f=float(frame-std::floor(frame));
    std::array<float,10>a,b,result;for(int c=0;c<10;c++){auto&v=t.channels[c];a[c]=v[v.size()==1?0:lo];b[c]=v[v.size()==1?0:hi];result[c]=a[c]*(1-f)+b[c]*f;}
    // PMQuaternion::lerpTo uses shortest-path spherical interpolation, with a linear near-angle branch.
    float dot=0;for(int c=3;c<=6;c++)dot+=a[c]*b[c];if(dot<0){dot=-dot;for(int c=3;c<=6;c++)b[c]=-b[c];}
    float left=1-f,right=f;if(1-dot>1e-7f){float angle=std::acos(std::clamp(dot,-1.f,1.f));float sine=std::sin(angle);left=std::sin((1-f)*angle)/sine;right=std::sin(f*angle)/sine;}
    for(int c=3;c<=6;c++)result[c]=left*a[c]+right*b[c];return result;
}
static void pose(double ms){
    for(size_t i=0;i<model.nodes.size();i++){auto&n=model.nodes[i];Mat local=transform(n.frame);
        if(!bindPose&&!animation.tracks.empty())local=multiply(local,transform(sample(animation.tracks[i],ms,animation.frameMS)));
        n.world=multiply(n.parent<0?identity():model.nodes[n.parent].world,local);}
}
static Model loadModel(const Entry&e){
    std::ifstream in(e.path);std::string magic;size_t nn,np;Model m;
    require(bool(in>>magic>>m.id>>nn>>np)&&magic=="TPM1"&&m.id==e.id&&nn>0&&nn<=1024&&np>0&&np<=4096,"Invalid model header");
    for(size_t i=0;i<nn;i++){Node n;char tag;require(bool(in>>tag>>n.parent)&&tag=='N'&&n.parent>=-1&&n.parent<int(i),"Invalid model hierarchy");
        n.locator=locatorCatalog[m.id].count(int(i))!=0;
        for(float&v:n.frame)require(bool(in>>v)&&std::isfinite(v)&&std::abs(v)<1e8,"Invalid transform");
        n.world=multiply(n.parent<0?identity():m.nodes[n.parent].world,transform(n.frame));m.nodes.push_back(n);}
    std::array<float,3> lo={1e30f,1e30f,1e30f},hi={-1e30f,-1e30f,-1e30f};
    size_t total=0;
    for(size_t i=0;i<np;i++){Part p;char tag;size_t nv;
        require(bool(in>>tag>>p.node>>p.mesh>>p.alpha>>nv>>p.texture)&&tag=='P'&&p.node>=0&&p.node<int(nn)&&nv>0&&nv%3==0&&nv<=3000000&&std::isfinite(p.alpha)&&p.alpha>=0&&p.alpha<=1,"Invalid mesh header");
        total+=nv;require(total<=6000000,"Model too large");if(p.texture!="-")require(fs::is_regular_file(assetPath(p.texture)),"Missing model texture");
        p.vertices.resize(nv);
        for(auto&v:p.vertices){require(bool(in>>v.x>>v.y>>v.z>>v.u>>v.v),"Truncated mesh");for(float f:{v.x,v.y,v.z,v.u,v.v})require(std::isfinite(f)&&std::abs(f)<1e8,"Invalid vertex");
            auto pos=point(m.nodes[p.node].world,v);for(int k=0;k<3;k++){require(std::isfinite(pos[k]),"Invalid world transform");lo[k]=std::min(lo[k],pos[k]);hi[k]=std::max(hi[k],pos[k]);}}
        m.triangles+=nv/3;m.parts.push_back(std::move(p));}
    require(m.triangles==e.triangles,"Triangle count mismatch");std::string extra;require(!(in>>extra),"Trailing model data");
    float size=0;for(int k=0;k<3;k++){m.center[k]=(lo[k]+hi[k])*0.5f;size+=(hi[k]-lo[k])*(hi[k]-lo[k]);}m.radius=std::max(0.01f,std::sqrt(size)*0.5f);return m;
}
static GLuint texture(const std::string&path){
    if(path=="-")return 0;auto it=textures.find(path);if(it!=textures.end())return it->second;
    Gdiplus::Bitmap bitmap(assetPath(path).c_str());int w=bitmap.GetWidth(),h=bitmap.GetHeight();
    require(bitmap.GetLastStatus()==Gdiplus::Ok&&w>0&&h>0&&w<=8192&&h<=8192,"PNG load failed");
    Gdiplus::BitmapData data{};Gdiplus::Rect rect(0,0,w,h);
    require(bitmap.LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&data)==Gdiplus::Ok,"PNG lock failed");
    std::vector<unsigned char> rgba(size_t(w)*h*4);
    // Keep source scanline order: the original GLES loader uploads the same bytes.
    for(int y=0;y<h;y++){auto row=(unsigned char*)data.Scan0+y*data.Stride;for(int x=0;x<w;x++){size_t j=(size_t(y)*w+x)*4;rgba[j]=row[x*4+2];rgba[j+1]=row[x*4+1];rgba[j+2]=row[x*4];rgba[j+3]=row[x*4+3];}}
    bitmap.UnlockBits(&data);GLuint id;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    const char*extensions=reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));if(extensions&&std::string(extensions).find("GL_EXT_texture_filter_anisotropic")!=std::string::npos){float limit=1;glGetFloatv(0x84FF,&limit);glTexParameterf(GL_TEXTURE_2D,0x84FE,std::min(limit,4.f));}
    // PMImageManager::loadFor3D overrides the 2D loader's clamp with repeat.
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    // Original loadFor3D requests mipmaps (0x8191); build them portably for GL 1.1.
    for(int level=0;;level++){
        glTexImage2D(GL_TEXTURE_2D,level,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,rgba.data());
        if(w==1&&h==1)break;int nw=std::max(1,w/2),nh=std::max(1,h/2);std::vector<unsigned char> down(size_t(nw)*nh*4);
        for(int y=0;y<nh;y++)for(int x=0;x<nw;x++)for(int c=0;c<4;c++){
            unsigned sum=0;for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++)sum+=rgba[(size_t(std::min(h-1,y*2+dy))*w+std::min(w-1,x*2+dx))*4+c];
            down[(size_t(y)*nw+x)*4+c]=static_cast<unsigned char>((sum+2)/4);}
        rgba=std::move(down);w=nw;h=nh;
    }
    require(glGetError()==GL_NO_ERROR,"OpenGL texture upload failed");textures[path]=id;return id;
}
static void resetCamera(){yaw=-35;pitch=park?35:62;zoom=1;panX=panY=0;}
static void clearSceneMeshes(){for(auto&kv:sceneMeshes)if(kv.second.list)glDeleteLists(kv.second.list,1);sceneMeshes.clear();}
static void loadScene(){
    if(!scene.empty())return;
    std::ifstream in(root/L"extracted/runtime/park_scene.tsv");std::string magic;size_t count;
    require(bool(in>>magic>>count)&&magic=="TPS1"&&count>0&&count<=100000,"Run tools/export_park_scene.py first; invalid scene header");
    std::vector<SceneInstance> pending;std::array<float,2> lo={1e30f,1e30f},hi={-1e30f,-1e30f};bool hasGround=false;
    for(size_t j=0;j<count;j++){SceneInstance i;
        require(bool(in>>i.kind>>i.row>>i.model>>i.x>>i.y>>i.z>>i.angle>>i.texture)&&(i.kind=='G'||i.kind=='W'||i.kind=='P')&&i.row>=0,"Invalid scene instance");
        for(float v:{i.x,i.y,i.z,i.angle})require(std::isfinite(v)&&std::abs(v)<1e7,"Invalid scene coordinate");
        require(std::any_of(catalog.begin(),catalog.end(),[&](const Entry&e){return e.id==i.model;}),"Scene model missing");
        if(i.texture!="-")require(fs::is_regular_file(assetPath(i.texture)),"Scene texture missing");
        if(i.kind=='G'){require(i.model==392,"Unsupported ground bounds");hasGround=true;lo[0]=std::min(lo[0],i.x-576);hi[0]=std::max(hi[0],i.x+576);lo[1]=std::min(lo[1],i.y-576);hi[1]=std::max(hi[1],i.y+576);}
        pending.push_back(i);
    }
    require(!(in>>magic)&&hasGround,"Invalid scene bounds/trailing data");
    sceneCenter={(lo[0]+hi[0])/2,(lo[1]+hi[1])/2,0};sceneRadius=std::hypot(hi[0]-lo[0],hi[1]-lo[1])/2;scene=std::move(pending);
}
static bool waterEffect(const Model&m,const Part&p){return m.id==425&&((p.node>=10&&p.node<=18)||(p.node>=23&&p.node<=30)||(p.node>=33&&p.node<=35)||p.node==38);}
static std::map<int,GLuint> geometryLists;
static void emitPart(const Part&p,bool water){glBegin(GL_TRIANGLES);for(size_t vi=0;vi<p.vertices.size();vi++){const auto&v=p.vertices[vi];if(vi%3==0){const auto&b=p.vertices[vi+1];const auto&c=p.vertices[vi+2];float x=b.x-v.x,y=b.y-v.y,z=b.z-v.z,u=c.x-v.x,w=c.y-v.y,t=c.z-v.z;glNormal3f(y*t-z*w,z*u-x*t,x*w-y*u);}glTexCoord2f(v.u,v.v+(water&&v.u<.23f?float(std::fmod(waterFXTime*.45,1.)):0));glVertex3f(v.x,v.y,v.z);}glEnd();}
static void drawParts(const Model&m,const std::string&overrideTexture="-"){
    GLboolean depthWrite;glGetBooleanv(GL_DEPTH_WRITEMASK,&depthWrite);
    // Authored water/splash alpha must composite over the finished pool, not grass.
    for(int pass=0;pass<(m.id==425?2:1);pass++){if(pass==1)glDepthMask(GL_FALSE);
    for(auto&p:m.parts){if(waterEffect(m,p)!=(pass==1))continue;if(m.nodes[p.node].locator)continue;GLuint tex=texture(overrideTexture=="-"?p.texture:overrideTexture);
        if(tex&&!wire){glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,tex);}else glDisable(GL_TEXTURE_2D);
        glColor4f(1,1,1,p.alpha);glPushMatrix();glMultMatrixf(m.nodes[p.node].world.data());
        bool water=waterEffect(m,p);auto cached=geometryLists.find(p.mesh);if(!water&&cached!=geometryLists.end())glCallList(cached->second);else{GLint compiling=0;glGetIntegerv(GL_LIST_INDEX,&compiling);if(!water&&!compiling){GLuint list=glGenLists(1);glNewList(list,GL_COMPILE);emitPart(p,false);glEndList();geometryLists[p.mesh]=list;glCallList(list);}else emitPart(p,water);}glPopMatrix();}}glDepthMask(depthWrite);
}
static void prepareScene(){
    loadScene();
    for(auto&i:scene){auto key=std::make_pair(i.model,i.texture);if(sceneMeshes.count(key))continue;
        auto entry=std::find_if(catalog.begin(),catalog.end(),[&](const Entry&e){return e.id==i.model;});
        SceneMesh mesh;mesh.model=loadModel(*entry);
        // Upload before compiling: glTexImage2D must not be captured in a list.
        for(auto&p:mesh.model.parts)texture(i.texture=="-"?p.texture:i.texture);
        mesh.list=glGenLists(1);require(mesh.list!=0,"Scene display list allocation failed");
        glNewList(mesh.list,GL_COMPILE);drawParts(mesh.model,i.texture);glEndList();sceneMeshes.emplace(key,std::move(mesh));
    }
    for(auto& instance:scene){
        const Model& mesh=sceneMeshes.at({instance.model,instance.texture}).model;
        instance.cullRadius=mesh.radius+std::hypot(mesh.center[0],mesh.center[1],mesh.center[2]);
    }
}
static void setPark(bool enabled){
    if(enabled)prepareScene();park=enabled;resetCamera();
    SetWindowTextW(window,park?L"Theme Park | Original static park | Native Windows workbench":L"Theme Park | Original 3D model | Native Windows workbench");
}
static void selectModel(int next){
    next=(next+int(catalog.size()))%int(catalog.size());Model replacement=loadModel(catalog[next]);
    clearSceneMeshes();park=false;
    for(auto&kv:textures)glDeleteTextures(1,&kv.second);textures.clear();
    auto nextAnimation=loadAnimation(replacement.id,replacement.nodes.size());
    for(auto&p:replacement.parts)texture(p.texture);model=std::move(replacement);animation=std::move(nextAnimation);animationMS=0;lastTick=GetTickCount64();selected=next;resetCamera();
    auto title=L"Theme Park | Original 3D model "+std::to_wstring(model.id)+L" | Native Windows workbench";SetWindowTextW(window,title.c_str());
}
static void text(int x,int y,const std::string&s,float r=.83f,float g=.88f,float b=.94f){glColor3f(r,g,b);glRasterPos2i(x,y);glListBase(fontBase);glCallLists(GLsizei(s.size()),GL_UNSIGNED_BYTE,s.data());}
#include "park_game.h"
static void startupRender(bool present=true){
    glViewport(0,0,width,height);glClearColor(.025f,.25f,.34f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);glDisable(GL_DEPTH_TEST);glDisable(GL_LIGHTING);glDisable(GL_ALPHA_TEST);glDisable(GL_TEXTURE_2D);glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,width,height,0,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);int cx=width/2,cy=height/2;uiPanel(cx-270,cy-235,540,450,true);uiPanel(cx-230,cy+158,460,50);glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,texture("runtime/icon.png"));glColor4f(1,1,1,1);glBegin(GL_QUADS);glTexCoord2f(0,0);glVertex2i(cx-74,cy-202);glTexCoord2f(1,0);glVertex2i(cx+74,cy-202);glTexCoord2f(1,1);glVertex2i(cx+74,cy-54);glTexCoord2f(0,1);glVertex2i(cx-74,cy-54);glEnd();glDisable(GL_TEXTURE_2D);
    uiText(cx-250,cy-15,"THEME PARK",1,.72f,.05f,34,500);uiText(cx-250,cy+28,"BUILD. PLAY. GROW.",1,1,1,20,500);gameRect(cx-210,cy+67,420,14,.02f,.22f,.3f);gameRect(cx-210,cy+67,int(420*startupProgress),14,1,.76f,.1f);uiText(cx-250,cy+122,startupReady?"Your park is ready!":startupMessage,1,1,1,19,500);uiText(cx-230,cy+192,startupReady?"ENTER PARK  [ENTER]":"LOADING YOUR PARK...",.02f,.4f,.5f,23,460);glFlush();if(present)SwapBuffers(dc);
}
static void render(bool present=true){
    if(startupActive){startupRender(present);return;}
    pose(animationMS);
    if(game&&selectedGuest>=0){const auto&g=sim.guests[selectedGuest];auto p=sim.position(g);sceneCenter={float(p.first),float(p.second),0};if(g.state==playtest::State::Riding&&sim.types[sim.rides[g.target].type].model==404){const auto&r=sim.rides[g.target];auto it=std::find(r.riders.begin(),r.riders.end(),selectedGuest);auto v=castleRider(r,int(it-r.riders.begin()),g.wait);sceneCenter={v.x,v.y,v.z};}}
    if(game)gameClampCamera();
    glViewport(0,0,width,height);if(game){float d=gameDaylight();waterFXTime=sim.time;glClearColor(.08f+.78f*d,.12f+.82f*d,.25f+.75f*d,1);}else glClearColor(.055f,.075f,.10f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();float extent=1.35f/zoom,aspect=float(width)/height;glOrtho(-extent*aspect,extent*aspect,-extent,extent,-20,20);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();glTranslatef(panX,panY,0);glRotatef(-pitch,1,0,0);glRotatef(yaw,0,0,1);
    glEnable(GL_DEPTH_TEST);glDisable(GL_BLEND);glDisable(GL_TEXTURE_2D);glDisable(GL_ALPHA_TEST);
    if(grid){glColor3f(.14f,.19f,.23f);glBegin(GL_LINES);for(int i=-10;i<=10;i++){float p=i*.2f;glVertex3f(-2,p,-1.05f);glVertex3f(2,p,-1.05f);glVertex3f(p,-2,-1.05f);glVertex3f(p,2,-1.05f);}glEnd();}
    float radius=park?sceneRadius:model.radius;const auto&center=park?sceneCenter:model.center;
    glScalef(1/radius,1/radius,1/radius);glTranslatef(-center[0],-center[1],-center[2]);
    glEnable(GL_ALPHA_TEST);glAlphaFunc(GL_GREATER,.02f);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK,wire?GL_LINE:GL_FILL);
    if(game)gameLighting();
    if(park){for(const auto&i:scene){if(game&&!gameVisible(i.x,i.y,i.cullRadius,i.z))continue;if(game&&i.kind=='P'&&i.x>=89*32&&i.x<=94*32&&i.y<-142.5f*32&&i.y>=-146*32)continue;if((i.kind=='G'&&!terrain)||(i.kind=='W'&&!walls)||(i.kind=='P'&&!paths))continue;
        glPushAttrib(GL_ENABLE_BIT);if(game&&i.kind=='W'){GLdouble plane[]={0,1,0,145.25*32};glClipPlane(GL_CLIP_PLANE0,plane);glEnable(GL_CLIP_PLANE0);}
        glPushMatrix();glTranslatef(i.x,i.y,i.z);glRotatef(i.angle,0,0,1);if(game)gameSceneItem(i);else glCallList(sceneMeshes.at({i.model,i.texture}).list);glPopMatrix();glPopAttrib();}}
    else drawParts(model);
    if(game){
        gameWorld();
        // Draw floating banks after the entire park. Only their anchors belong
        // to locked land; the silhouette must be free to cross its ground edge.
        gameClouds();
    }
    glDisable(GL_LIGHTING);for(int n=0;n<8;n++)glDisable(GL_LIGHT0+n);glDisable(GL_NORMALIZE);glDisable(GL_COLOR_MATERIAL);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glDisable(GL_TEXTURE_2D);glDisable(GL_ALPHA_TEST);glDisable(GL_BLEND);glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,width,height,0,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();
    if(game){gameHUD();glFlush();require(glGetError()==GL_NO_ERROR,"Playtest render error");if(present)SwapBuffers(dc);return;}
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glColor4f(.025f,.04f,.055f,.88f);
    glBegin(GL_QUADS);for(auto bounds:std::array<std::array<int,2>,2>{{{0,130},{height-88,height}}}){
        glVertex2i(0,bounds[0]);glVertex2i(width,bounds[0]);glVertex2i(width,bounds[1]);glVertex2i(0,bounds[1]);}glEnd();glDisable(GL_BLEND);
    text(28,35,park?"THEME PARK / ORIGINAL STATIC LAYOUT":"THEME PARK / ORIGINAL 3D",.39f,.83f,.73f);
    if(park){
        text(28,62,std::to_string(scene.size())+" instances   |   "+std::to_string(sceneMeshes.size())+" cached assemblies");
        text(28,87,"1: grass ["+std::string(terrain?"ON":"OFF")+"]   2: walls ["+(walls?"ON":"OFF")+"]   3: paths ["+(paths?"ON":"OFF")+"]");
        text(28,112,"Default terrain/path theme. Owned rides and park progression are unresolved.");
    }else{
    text(28,62,"Model "+std::to_string(model.id)+"     "+std::to_string(selected+1)+" / "+std::to_string(catalog.size()));
    text(28,87,std::to_string(model.triangles)+" triangles   |   "+std::to_string(model.nodes.size())+" nodes   |   "+std::to_string(textures.size())+" textures");
    text(28,112,animation.group<0?"No verified animation binding":("Original idle group "+std::to_string(animation.group)+"   |   "+(bindPose?"BIND POSE":playing?"PLAYING":"PAUSED")+"   |   "+std::to_string(int(animationMS))+" ms"));
    }
    text(28,height-66,"Drag: orbit    Right-drag: pan    Wheel: zoom    Home: reset");
    text(28,height-42,park?"P: model viewer   1 / 2 / 3: layers   W: wireframe   G: grid":"Left / Right: model   A: next animated   Space: pause   B: bind pose   W: wireframe");
    text(28,height-18,"P: park / model   [ / ]: frame step   Esc: exit   |   Original data preview; no gameplay.",.48f,.57f,.64f);
    glFlush();require(glGetError()==GL_NO_ERROR,"OpenGL rendering error");if(present)SwapBuffers(dc);
}
static void saveFrame(const fs::path&path,int width,int height){
    std::vector<unsigned char> pixels(size_t(width)*height*4);glReadBuffer(GL_BACK);glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
    Gdiplus::Bitmap bmp(width,height,PixelFormat32bppARGB);Gdiplus::BitmapData data{};Gdiplus::Rect rect(0,0,width,height);
    require(bmp.LockBits(&rect,Gdiplus::ImageLockModeWrite,PixelFormat32bppARGB,&data)==Gdiplus::Ok,"Screenshot lock failed");
    size_t visible=0;for(int y=0;y<height;y++){auto dst=(unsigned char*)data.Scan0+y*data.Stride;auto src=pixels.data()+size_t(height-1-y)*width*4;
        for(int x=0;x<width;x++){dst[x*4]=src[x*4+2];dst[x*4+1]=src[x*4+1];dst[x*4+2]=src[x*4];dst[x*4+3]=255;if(src[x*4]>65||src[x*4+1]>65||src[x*4+2]>65)visible++;}}
    bmp.UnlockBits(&data);require(visible>5000,"Empty render");CLSID png={0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0,0,0xf8,0x1e,0xf3,0x2e}};
    require(bmp.Save(path.c_str(),&png,nullptr)==Gdiplus::Ok,"Screenshot save failed");
}
static void capture(const fs::path&path){render(false);saveFrame(path,width,height);}
static void exportShopArt(){
    auto dir=root/L"extracted/runtime/shop_art";fs::create_directories(dir);
    for(const auto&t:sim.types){
        if(t.model!=414&&t.model!=415&&t.model!=423)continue;
        glViewport(0,0,512,384);glClearColor(.35f,.65f,.24f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        uiModel(t.model,0,height-384,512,384,true);
        saveFrame(dir/(std::to_string(t.model)+".png"),512,384);
    }
}
static bool gamePointerPending=false;static POINT gamePointerStart{};
static LRESULT CALLBACK proc(HWND h,UINT msg,WPARAM w,LPARAM l){
    try{if(game&&cameraBusy()&&(msg==WM_KEYDOWN||msg==WM_MOUSEMOVE||msg==WM_MOUSEWHEEL||msg==WM_RBUTTONDOWN||msg==WM_LBUTTONDOWN||msg==WM_LBUTTONUP))return 0;switch(msg){
    case WM_GETMINMAXINFO:if(game){auto info=reinterpret_cast<MINMAXINFO*>(l);info->ptMinTrackSize={1180,840};return 0;}break;
    case WM_ACTIVATEAPP:if(game&&!testing){music.focused=w!=0;music.apply();sfx.focused=w!=0;if(!sfx.focused)sfx.stop();}break;
    case WM_CLOSE:if(game&&!testing){try{gameSave();}catch(const std::exception&e){gameNotice=std::string("Save failed: ")+e.what();InvalidateRect(h,nullptr,FALSE);return 0;}}DestroyWindow(h);return 0;
    case WM_SIZE:width=std::max(1,int(LOWORD(l)));height=std::max(1,int(HIWORD(l)));InvalidateRect(h,nullptr,FALSE);return 0;
    case WM_PAINT:{PAINTSTRUCT p;BeginPaint(h,&p);if(context&&(startupActive||!model.parts.empty()))render();EndPaint(h,&p);return 0;}
    case WM_ERASEBKGND:return 1;
    case WM_TIMER:if(startupActive)return 0;{ULONGLONG now=GetTickCount64();if(game){gameTick(double(now-lastTick)/1000);InvalidateRect(h,nullptr,FALSE);}else if(!park&&playing&&!bindPose&&!animation.tracks.empty()){animationMS+=double(now-lastTick);InvalidateRect(h,nullptr,FALSE);}lastTick=now;return 0;}
    case WM_LBUTTONDOWN:case WM_RBUTTONDOWN:if(startupActive){if(startupReady&&msg==WM_LBUTTONDOWN&&GET_X_LPARAM(l)>width/2-270&&GET_X_LPARAM(l)<width/2+270&&GET_Y_LPARAM(l)>height/2+165&&GET_Y_LPARAM(l)<height/2+215){startupActive=false;lastTick=GetTickCount64();InvalidateRect(h,nullptr,FALSE);}return 0;}if(game&&inspectionOpen()){if(msg==WM_LBUTTONDOWN)gameClick(GET_X_LPARAM(l),GET_Y_LPARAM(l));InvalidateRect(h,nullptr,FALSE);return 0;}if(game&&msg==WM_LBUTTONDOWN){int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l);for(const auto&b:gameButtons)if(x>=b.x&&x<b.x+b.w&&y>=b.y&&y<b.y+b.h){gameClick(x,y);InvalidateRect(h,nullptr,FALSE);return 0;}gamePointerPending=true;gamePointerStart=mouse={x,y};drag=false;pan=true;SetCapture(h);return 0;}if(game){selectedGuest=-1;rideCamera.active=false;}drag=true;pan=msg==WM_RBUTTONDOWN;mouse={GET_X_LPARAM(l),GET_Y_LPARAM(l)};SetCapture(h);return 0;
    case WM_LBUTTONUP:case WM_RBUTTONUP:if(gamePointerPending&&!drag){gameClick(gamePointerStart.x,gamePointerStart.y);InvalidateRect(h,nullptr,FALSE);}gamePointerPending=false;drag=false;ReleaseCapture();return 0;
    case WM_CAPTURECHANGED:gamePointerPending=false;drag=false;return 0;
    case WM_MOUSEMOVE:if(game&&inspectionOpen())return 0;if(gamePointerPending&&!drag&&std::hypot(double(GET_X_LPARAM(l)-gamePointerStart.x),double(GET_Y_LPARAM(l)-gamePointerStart.y))>5){drag=true;pan=true;selectedGuest=-1;rideCamera.active=false;}if(game){int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l);hoverSlot=gamePanel(x,y)?-1:gamePickSlot(x,y);}if(drag){int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l);if(pan){panX+=(x-mouse.x)*2.7f/height/zoom;panY-=(y-mouse.y)*2.7f/height/zoom;}else{yaw+=(x-mouse.x)*.5f;pitch=std::clamp(pitch+(y-mouse.y)*.5f,5.f,175.f);}mouse={x,y};InvalidateRect(h,nullptr,FALSE);}return 0;
    case WM_MOUSEWHEEL:if(game&&inspectionOpen())return 0;if(game)rideCamera.active=false;if(game)gameZoomTarget=std::clamp(gameZoomTarget*std::pow(1.15f,float(GET_WHEEL_DELTA_WPARAM(w))/120),4.f,7.f);else zoom=std::clamp(zoom*std::pow(1.15f,float(GET_WHEEL_DELTA_WPARAM(w))/120),.1f,20.f);InvalidateRect(h,nullptr,FALSE);return 0;
    case WM_KEYDOWN:if(startupActive){if(startupReady&&w==VK_RETURN){startupActive=false;lastTick=GetTickCount64();InvalidateRect(h,nullptr,FALSE);}return 0;}
        if(game){if(inspectionOpen()&&(w=='B'||w=='O'||w=='Q'||w=='E'||w==VK_HOME||w=='R'))return 0;if(w==VK_F1)gameAction(36);if(w=='B')gameAction(30);if(w=='G')gameAction(37);if(w=='O'){selectedGuest=-1;sceneCenter={91*32.f,-140*32.f,0};zoom=gameZoomTarget=4;panX=panY=0;}if(w==VK_ESCAPE){shopOpen=false;gameAction(19);}if(w==VK_SPACE)gameAction(10);if(w=='R')gameAction(14);if(w=='Q'){rideCamera.active=false;yaw-=15;}if(w=='E'){rideCamera.active=false;yaw+=15;}if(w=='S')gameAction(17);if(w=='L')gameAction(18);if(w==VK_HOME&&selectedRide>=0)gameFocus(sim.rides[selectedRide].slot);InvalidateRect(h,nullptr,FALSE);return 0;}
        if(w==VK_ESCAPE){DestroyWindow(h);return 0;}if(w==VK_RIGHT)selectModel(selected+1);if(w==VK_LEFT)selectModel(selected-1);
        if(w=='A'){for(int i=1;i<=int(catalog.size());i++){int next=(selected+i)%int(catalog.size());if(animationCatalog.count(catalog[next].id)){selectModel(next);break;}}}
        if(w==VK_SPACE)playing=!playing;if(w=='B')bindPose=!bindPose;
        if(w==VK_OEM_4||w==VK_OEM_6){playing=false;animationMS=std::max(0.,animationMS+(w==VK_OEM_6?33:-33));}
        if(w=='P')setPark(!park);
        if(park){if(w=='1')terrain=!terrain;if(w=='2')walls=!walls;if(w=='3')paths=!paths;}
        if(w==VK_HOME)resetCamera();if(w=='W'){wire=!wire;clearSceneMeshes();if(park)prepareScene();}if(w=='G')grid=!grid;InvalidateRect(h,nullptr,FALSE);return 0;
    case WM_DESTROY:KillTimer(h,1);PostQuitMessage(0);return 0;
    }}catch(const std::exception&e){if(!testing)MessageBoxA(h,e.what(),"3D workbench error",MB_ICONERROR);PostQuitMessage(2);}
    return DefWindowProcW(h,msg,w,l);
}
static void initGL(){
    dc=GetDC(window);PIXELFORMATDESCRIPTOR pfd{};pfd.nSize=sizeof(pfd);pfd.nVersion=1;pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;pfd.iPixelType=PFD_TYPE_RGBA;pfd.cColorBits=32;pfd.cDepthBits=24;pfd.cStencilBits=8;
    int fmt=ChoosePixelFormat(dc,&pfd);
    // Probe optional WGL MSAA using a private bootstrap window; never reassign a window's pixel format.
    HWND probe=CreateWindowExW(0,L"STATIC",L"",WS_POPUP,0,0,1,1,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    if(probe){HDC pd=GetDC(probe);int pf=ChoosePixelFormat(pd,&pfd);HGLRC pc=nullptr;
        if(pf&&SetPixelFormat(pd,pf,&pfd))pc=wglCreateContext(pd);
        if(pc&&wglMakeCurrent(pd,pc)){using Choose=BOOL(WINAPI*)(HDC,const int*,const FLOAT*,UINT,int*,UINT*);auto choose=reinterpret_cast<Choose>(wglGetProcAddress("wglChoosePixelFormatARB"));
            if(choose){int attributes[]={0x2001,1,0x2010,1,0x2011,1,0x2013,0x202B,0x2014,32,0x2022,24,0x2023,8,0x2041,1,0x2042,4,0};int candidate=0;UINT found=0;if(choose(dc,attributes,nullptr,1,&candidate,&found)&&found&&candidate){fmt=candidate;multisampling=true;}}
            wglMakeCurrent(nullptr,nullptr);}
        if(pc)wglDeleteContext(pc);ReleaseDC(probe,pd);DestroyWindow(probe);}
    require(fmt&&SetPixelFormat(dc,fmt,&pfd),"Pixel format setup failed");context=wglCreateContext(dc);require(context&&wglMakeCurrent(dc,context),"OpenGL context failed");if(multisampling)glEnable(0x809D);
    HFONT font=CreateFontW(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,FF_DONTCARE,L"Consolas");
    HGDIOBJ old=SelectObject(dc,font);fontBase=glGenLists(256);require(fontBase&&wglUseFontBitmapsA(dc,0,256,fontBase),"Font setup failed");SelectObject(dc,old);DeleteObject(font);
}
static void verify(){
    auto out=root/L"analysis/runtime_test";fs::create_directories(out);size_t triangles=0,parts=0;int count=0;
    for(auto&e:catalog){Model m=loadModel(e);triangles+=m.triangles;parts+=m.parts.size();count++;}
    selectModel(409<int(catalog.size())?409:0);capture(out/L"native-3d-front.png");float y0=yaw,p0=pitch;
    SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(200,200));SendMessageW(window,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(420,230));SendMessageW(window,WM_LBUTTONUP,0,0);
    require(yaw!=y0&&pitch!=p0,"Orbit input failed");capture(out/L"native-3d-back.png");
    SendMessageW(window,WM_MOUSEWHEEL,MAKEWPARAM(0,120),0);require(zoom>1,"Zoom input failed");
    SendMessageW(window,WM_RBUTTONDOWN,MK_RBUTTON,MAKELPARAM(200,200));SendMessageW(window,WM_MOUSEMOVE,MK_RBUTTON,MAKELPARAM(240,220));SendMessageW(window,WM_RBUTTONUP,0,0);require(panX!=0&&panY!=0,"Pan input failed");
    SendMessageW(window,WM_KEYDOWN,VK_HOME,0);require(zoom==1&&panX==0&&panY==0,"Reset failed");
    SendMessageW(window,WM_KEYDOWN,'W',0);require(wire,"Wireframe input failed");capture(out/L"native-3d-wireframe.png");SendMessageW(window,WM_KEYDOWN,'W',0);
    int previous=model.id;SendMessageW(window,WM_KEYDOWN,VK_RIGHT,0);require(model.id!=previous,"Model navigation failed");SendMessageW(window,WM_KEYDOWN,VK_LEFT,0);require(model.id==previous,"Reverse navigation failed");
    // Exercise every original assembly and texture through real GL uploads/draws.
    for(int i=0;i<int(catalog.size());i++){selectModel(i);render(false);}
    int animated=0;for(auto&binding:animationCatalog){auto it=std::find_if(catalog.begin(),catalog.end(),[&](auto&e){return e.id==binding.first;});require(it!=catalog.end(),"Animation model missing");selectModel(int(it-catalog.begin()));pose(0);std::vector<Mat> first;for(auto&n:model.nodes)first.push_back(n.world);animationMS=330;pose(animationMS);
        bool moved=false;for(size_t i=0;i<first.size();i++)for(int k=0;k<16;k++)if(std::abs(first[i][k]-model.nodes[i].world[k])>1e-5)moved=true;require(moved,"Original animation did not move");
        for(int frame=0;frame<60;frame++){animationMS=frame*16.5;render(false);}animated++;}
    selectModel(425);animationMS=0;capture(out/L"native-animation-0.png");animationMS=330;capture(out/L"native-animation-330.png");
    playing=true;SendMessageW(window,WM_KEYDOWN,VK_SPACE,0);require(!playing,"Animation pause failed");double before=animationMS;SendMessageW(window,WM_KEYDOWN,VK_OEM_6,0);require(animationMS==before+33,"Frame step failed");SendMessageW(window,WM_KEYDOWN,'B',0);require(bindPose,"Bind pose input failed");SendMessageW(window,WM_KEYDOWN,'B',0);SendMessageW(window,WM_KEYDOWN,'A',0);require(model.id==427,"Animated navigation failed");
    SendMessageW(window,WM_KEYDOWN,'P',0);require(park&&scene.size()==5934,"Park scene load failed");
    capture(out/L"native-park-overview.png");yaw=0;pitch=0;zoom=7;capture(out/L"native-park-detail.png");
    for(int key='1';key<='3';key++){SendMessageW(window,WM_KEYDOWN,key,0);render(false);SendMessageW(window,WM_KEYDOWN,key,0);}
    require(terrain&&walls&&paths,"Park layer toggle failed");
    SendMessageW(window,WM_KEYDOWN,'W',0);require(wire,"Park wireframe failed");render(false);SendMessageW(window,WM_KEYDOWN,'W',0);
    SendMessageW(window,WM_KEYDOWN,'P',0);require(!park,"Return to model failed");render(false);
    SendMessageW(window,WM_KEYDOWN,VK_RIGHT,0);SendMessageW(window,WM_KEYDOWN,'P',0);require(park&&!sceneMeshes.empty(),"Scene cache reload failed");render(false);
    std::ofstream report(out/L"model_test.json");report<<"{\n  \"passed\": true,\n  \"models_parsed_and_rendered\": "<<count<<",\n  \"parts\": "<<parts<<",\n  \"triangles\": "<<triangles<<",\n  \"camera_and_navigation\": true,\n  \"renderer\": \""<<glGetString(GL_RENDERER)<<"\",\n  \"animated_models_verified\": "<<animated<<",\n  \"park_instances\": "<<scene.size()<<",\n  \"park_cached_assemblies\": "<<sceneMeshes.size()<<",\n  \"park_layers_and_cache_reload\": true\n}\n";
}
static void verifyGame(){
    sim.adaptiveCrowd=false;
    debugHUD=true;
    sim.lockedSlots.clear();for(int t=0;t<20;t++)sim.openTerritories.insert(t);
    int matchingCatalog=0;for(size_t type=0;type<sim.types.size();type++){bool fits=false;for(size_t slot=0;slot<sim.slots.size();slot++)if(sim.canBuild(int(slot),int(type))){fits=true;break;}if(fits)matchingCatalog++;else require(sim.types[type].w==1||(sim.types[type].shop&&sim.types[type].w==16),"Unexpected unsupported ride footprint");}
    auto out=root/L"analysis/runtime_test";fs::create_directories(out);require(sim.guests.size()==4,"Initial guests missing");
    capture(out/L"playtest-start.png");
    for(int i=0;i<6000;i++){sim.tick(.05);if(i%100==0){sim.validate();render(false);}}
    sim.validate();require(sim.served>0,"Guests never completed a ride");capture(out/L"playtest-running.png");
    // UI buttons and world picking use the same messages as the visible app.
    SendMessageW(window,WM_KEYDOWN,VK_SPACE,0);require(sim.paused,"Pause key failed");double before=sim.time;sim.tick(.5);require(sim.time==before,"Paused clock advanced");SendMessageW(window,WM_KEYDOWN,VK_SPACE,0);
    render(false);auto button=std::find_if(gameButtons.begin(),gameButtons.end(),[](const GameButton&b){return b.action==100;});require(button!=gameButtons.end(),"Build card missing");
    SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(button->x+10,button->y+10));require(buildType==0,"Build card input failed");gameFindPlot();render(false);
    int plot=gamePickSlot(width/2,height/2);require(plot>=0&&sim.canBuild(plot,buildType,buildRotation),"Find plot/picking failed");size_t oldCount=sim.rides.size();
    SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(width/2,height/2));SendMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(width/2,height/2));require(sim.rides.size()==oldCount+1,"Mouse placement failed");require(!sim.canBuild(plot,0),"Duplicate build allowed");
    gameAction(15);require(!sim.rides[selectedRide].open,"Ride close failed");gameAction(15);require(sim.rides[selectedRide].open,"Ride reopen failed");
    gameSave();std::ostringstream state;sim.write(state);sim.tick(.05);gameLoad();std::ostringstream restored;sim.write(restored);require(state.str()==restored.str(),"Native save restore mismatch");
    auto guestCount=sim.guests.size();gameAction(12);require(sim.guests.size()==guestCount+8,"Add guests failed");gameAction(11);require(sim.speed==2,"Speed control failed");
    gameAction(20);require(selectedGuest>=0,"Guest inspector failed");capture(out/L"playtest-guest.png");gameAction(21);
    gameAction(100);gameFindPlot();hoverSlot=gamePickSlot(width/2,height/2);capture(out/L"playtest-build.png");gameAction(19);
    selectedRide=0;gameFocus(sim.rides[0].slot);for(int i=0;i<200;i++)sim.tick(.05);capture(out/L"playtest-running.png");
    int castle=-1;for(size_t i=0;i<sim.rides.size();i++)if(sim.types[sim.rides[i].type].model==404)castle=int(i);require(castle>=0,"Bouncy castle absent");
    for(int step=0;step<12000;step++){auto&r=sim.rides[castle];if(r.riders.size()>=2&&r.timer>6&&r.timer<11)break;sim.tick(.05);}
    auto&r=sim.rides[castle];require(r.riders.size()>=2&&r.timer>6&&r.timer<11,"Castle never boarded visitors");selectedRide=castle;gameFocus(r.slot);zoom=7;
    auto first=castleRider(r,0,sim.guests[r.riders[0]].wait);capture(out/L"castle-playing-0.png");sim.tick(.2);auto second=castleRider(r,0,sim.guests[r.riders[0]].wait);require(std::abs(first.z-second.z)>.01f,"Boarded visitor did not bounce");capture(out/L"castle-playing-1.png");
    auto boarding=r;boarding.timer=sim.types[r.type].duration;auto entry=castleRider(boarding,0);boarding.timer=0;auto exit=castleRider(boarding,0);require(std::abs(entry.x-exit.x)<.01&&std::abs(entry.y-exit.y)<.01,"Rider attachment moves through entrance");
    for(size_t i=0;i<sim.rides.size();i++)if(sim.types[sim.rides[i].type].model==425){selectedRide=int(i);gameFocus(sim.rides[i].slot);capture(out/L"water-slides-corrected.png");break;}
    debugHUD=false;selectedRide=castle;gameFocus(sim.rides[castle].slot);capture(out/L"original-ui-castle.png");shopOpen=true;shopSelection=4;capture(out/L"original-ui-rides.png");shopCategory=1;shopSelection=int(std::find(typeCategories.begin(),typeCategories.end(),1)-typeCategories.begin());capture(out/L"original-ui-shops.png");shopOpen=false;
    sceneCenter={91*32.f,-140*32.f,0};zoom=2;capture(out/L"original-entrance-office.png");
    int audioTracks=0;for(int i=1;i<=3;i++){require(music.open(assetPath("runtime/audio/track"+std::to_string(i)+".mp3"),true),"Original MP3 playback failed");wchar_t length[64]={};require(mciSendStringW(L"status themepark_music length",length,64,nullptr)==0&&_wtoi(length)>1000,"Music duration invalid");music.close();audioTracks++;}
    music.volume=0;gameAction(22);require(!music.enabled,"Music toggle failed");gameAction(22);require(music.enabled&&music.opened,"Music resume failed");gameAction(23);require(music.track==2&&music.opened,"Track change failed");music.close();music.volume=80;gameAction(24);require(music.volume==100,"Volume step failed");gameAction(24);require(music.volume==0,"Volume wrap failed");music.volume=20;
    std::ofstream(out/L"castle_audio_test.json")<<"{\"passed\":true,\"visible_riders\":"<<r.riders.size()<<",\"bounce_verified\":true,\"original_music_tracks_played_muted\":"<<audioTracks<<",\"msaa4\":"<<(multisampling?"true":"false")<<"}";
    // Exercise original-art menu controls through the native mouse handler.
    auto clickAction=[&](int action){render(false);auto it=std::find_if(gameButtons.begin(),gameButtons.end(),[&](const GameButton&b){return b.action==action;});require(it!=gameButtons.end(),"Original UI control missing");SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(it->x+it->w/2,it->y+it->h/2));};
    clickAction(30);clickAction(32);require(shopOpen&&shopCategory==1,"SHOPS tab failed");int firstShop=int(std::find(typeCategories.begin(),typeCategories.end(),1)-typeCategories.begin());clickAction(1000+firstShop);clickAction(35);require(!shopOpen&&buildType==firstShop,"Shop Build button failed");
    size_t shopCount=sim.rides.size();render(false);SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(width/2,height/2));SendMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(width/2,height/2));require(sim.rides.size()==shopCount+1&&sim.rides.back().type==firstShop,"Shop placement failed");
    SendMessageW(window,WM_KEYDOWN,'G',0);require(sim.rides.back().gateLevel==1,"Gate upgrade failed");gameSave();int upgraded=selectedRide;gameLoad();require(sim.rides[upgraded].gateLevel==1,"Gate upgrade not saved");selectedRide=upgraded;gameFocus(sim.rides[upgraded].slot);capture(out/L"original-shop-built.png");
    // Regression checks for the October gate/plot/camera requests.
    float savedYaw=yaw;zoom=gameZoomTarget=4;pitch=gameTilt(zoom);bool wasPaused=sim.paused;sim.paused=true;
    SendMessageW(window,WM_MOUSEWHEEL,MAKEWPARAM(0,120),0);require(zoom==4&&gameZoomTarget>zoom,"Wheel zoom must interpolate");
    gameTick(.05);require(zoom>4&&zoom<gameZoomTarget,"Smooth zoom failed");
    for(int i=0;i<100;i++)gameTick(.05);require(pitch>gameTilt(2)&&yaw==savedYaw,"Zoom tilt changed yaw or failed");
    gameFocus(sim.rides[castle].slot);require(yaw==savedYaw,"Focus changed yaw");sim.paused=wasPaused;
    auto&cr=sim.rides[castle];auto passengers=cr.riders;bool wasRunning=cr.running;cr.running=true;cr.riders.clear();gameDrawRide(cr.type,cr.slot,cr.rotation);auto resting=gameAssets.at(404).model.nodes[5].world;
    cr.riders=passengers;gameDrawRide(cr.type,cr.slot,cr.rotation);require(resting!=gameAssets.at(404).model.nodes[5].world,"Occupied castle did not animate");
    cr.riders.clear();gameDrawRide(cr.type,cr.slot,cr.rotation);require(resting==gameAssets.at(404).model.nodes[5].world,"Empty castle still animated");cr.riders=passengers;cr.running=wasRunning;
    int empty=-1;for(size_t i=0;i<sim.slots.size();i++)if(sim.occupant(int(i))<0&&sim.slots[i].w==4){empty=int(i);break;}require(empty>=0,"No test plot");
    sim.lockedSlots.insert(empty);gameFocus(empty);buildType=-1;shopOpen=false;render(false);
    SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(width/2,height/2));SendMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(width/2,height/2));require(shopOpen&&shopSlot==empty,"Plot did not open matching shop");
    capture(out/L"plot-locked-shop.png");clickAction(1000+shopSelection);clickAction(38);require(!sim.lockedSlots.count(empty),"Unlock button failed");
    for(int category=0;category<2;category++){shopCategory=category;render(false);for(auto&button:gameButtons)if(button.action>=1000)require(shopFits(button.action-1000),"Shop exposed wrong footprint");}
    capture(out/L"plot-matching-shop.png");shopOpen=false;shopSlot=-1;
    for(size_t i=0;i<sim.slots.size();i++)if(sim.occupant(int(i))<0&&sim.slots[i].id%3!=0)sim.lockedSlots.insert(int(i));
    sceneCenter={91*32.f,-140*32.f,0};zoom=gameZoomTarget=2;pitch=gameTilt(zoom);capture(out/L"entrance-and-plot-states.png");
    selectedRide=castle;gameFocus(sim.rides[castle].slot);capture(out/L"camera-near-tilt.png");
    int previousGate=sim.rides[castle].gateLevel;for(int level=0;level<4;level++){sim.rides[castle].gateLevel=level;capture(out/(L"aligned-gate-"+std::to_wstring(level)+L".png"));}sim.rides[castle].gateLevel=previousGate;
    zoom=gameZoomTarget=1.2f;pitch=gameTilt(zoom);capture(out/L"camera-far-tilt.png");
    sim.validate();gameSave();gameLoad();
    std::ofstream(out/L"refinement_test.json")<<"{\"passed\":true,\"instant_castle_attachment\":true,\"empty_castle_stops\":true,\"plot_unlock_click\":true,\"exact_size_shop_filter\":true,\"smooth_zoom_tilt_preserves_yaw\":true}";
    // Exercise economy and the ride-detail controls through native mouse hit regions.
    selectedRide=castle;rideDetails=true;shopOpen=false;gameFocus(sim.rides[castle].slot);sim.rides[castle].profit=120;sim.rides[castle].rewardFlags=8;
    capture(out/L"ride-detail-ready.png");int previousMoney=sim.money,previousFlags=sim.flags;clickAction(2000+castle);
    require(sim.money==previousMoney&&sim.flags==previousFlags&&!sim.loot.empty(),"Reward credited before pickup");sim.tick(.8);capture(out/L"reward-cash-flags.png");clickAction(4000);while(!sim.loot.empty())gameAction(4000);require(sim.money>previousMoney&&sim.flags==previousFlags+8&&sim.rides[castle].profit==0,"Native pickup failed");
    require(!sim.rides[castle].riders.empty(),"No castle guests for minigame test");for(int id:sim.rides[castle].riders)sim.guests[id].wait=sim.types[sim.rides[castle].type].duration;sim.rides[castle].timer=sim.types[sim.rides[castle].type].duration;
    clickAction(42);require(sim.rides[castle].challenge>=0,"Minigame button failed");double speedBefore=sim.rides[castle].tapSpeed;clickAction(42);require(sim.rides[castle].tapSpeed>speedBefore,"Taps did not accelerate ride");
    capture(out/L"ride-minigame.png");for(int i=0;i<401;i++){if(i>0&&i%11==0)gameAction(42);sim.tick(.05);}require(sim.rides[castle].challenge<0&&sim.streak>0&&sim.rides[castle].profit>=80,"Native minigame reward failed");
    auto keepPark=sim;sim.rides[castle].gateLevel=0;clickAction(37);require(gateFilm.slot>=0&&sim.rides[castle].gateLevel==0,"Gate changed before camera arrived");for(int i=0;i<52;i++){gameTick(.05);if(i==23)capture(out/L"gate-upgrade-reveal.png");}require(sim.rides[castle].gateLevel==1,"Cash gate upgrade button failed");clickAction(43);for(int i=0;i<52;i++)gameTick(.05);require(sim.rides[castle].gateLevel==2,"Ticket gate upgrade button failed");clickAction(41);require(sim.rides.size()+1==keepPark.rides.size()&&!rideDetails,"Sell button failed");sim.validate();sim=keepPark;selectedRide=castle;rideDetails=true;
    gameSave();gameLoad();require(sim.streak>0&&sim.flags>=8,"Economy save lost progress");rideDetails=false;
    auto allTerritories=sim.openTerritories;sim.openTerritories={17};for(auto&r:sim.rides)sim.openTerritories.insert(sim.territory(r.slot));
    sceneCenter={-20000,-20000,0};zoom=gameZoomTarget=2;pitch=gameTilt(zoom);panX=panY=0;gameClampCamera();require(std::abs(panX)>1||std::abs(panY)>1,"Camera escaped territory bounds");capture(out/L"cloud-horizon.png");
    sceneCenter={91*32.f,-140*32.f,0};panX=panY=0;capture(out/L"territory-clouds-and-rocks.png");
    {auto shot=currentShot();sceneCenter={73*32.f,-128*32.f,0};panX=panY=0;zoom=gameZoomTarget=4;pitch=gameTilt(zoom);capture(out/L"cloud-bank-boundary.png");blendShot(shot,shot,1);}

    territoryPanel=18;capture(out/L"territory-unlock.png");clickAction(48);require(territoryPanel<0,"Territory close failed");
    sim.openTerritories=allTerritories;selectedRide=castle;rideDetails=true;gameFocus(sim.rides[castle].slot);render(false);
    for(auto&b:gameButtons)require(b.action!=30&&b.action!=44&&b.action!=45&&b.action!=46,"Main HUD leaked into ride details");capture(out/L"ride-detail-clean.png");rideDetails=false;
    for(size_t i=0;i<sim.rides.size();i++)if(sim.types[sim.rides[i].type].model==427){selectedRide=int(i);gameFocus(sim.rides[i].slot);capture(out/L"skull-river-grounded.png");}
    {auto prior=sim;int riverType=1,riverSlot=-1;for(size_t i=0;i<sim.slots.size();i++)if(sim.canBuild(int(i),riverType)){riverSlot=int(i);break;}
        require(riverSlot>=0,"No river verification plot");int river=sim.build(riverSlot,riverType);selectedRide=river;gameFocus(riverSlot);zoom=gameZoomTarget=3;pitch=gameTilt(zoom);capture(out/L"skull-river-grounded.png");sim=prior;selectedRide=castle;}
    {auto prior=sim.openTerritories;sim.openTerritories={17};sceneCenter={91*32.f,-111*32.f,0};zoom=gameZoomTarget=2;pitch=gameTilt(zoom);panX=panY=0;capture(out/L"territory-boundary.png");sim.openTerritories=prior;gameFocus(sim.rides[castle].slot);}
    require(passengerPoses.at(521)==std::vector<int>({504,505,506,507})&&passengerPoses.at(503)==std::vector<int>({524,525,526,527}),"Special passenger poses missing");
    // Detail camera interpolates to the fixed gate edge; manual wheel cancels it.
    selectedRide=castle;gameFocus(sim.rides[castle].slot);float beforeYaw=yaw;bool pauseBefore=sim.paused;sim.paused=true;
    rideDetails=true;gameDetailFocus(sim.rides[castle].slot);require(yaw==beforeYaw&&rideCamera.active,"Detail camera snapped");gameTick(.15);require(rideCamera.active,"Detail camera ended early");
    for(int i=0;i<20;i++)gameTick(.05);require(!rideCamera.active&&std::abs(std::remainder(yaw+35,360.f))<.01f,"Camera did not face gate");rideDetails=true;capture(out/L"ride-gate-front.png");rideDetails=false;
    rideDetails=true;gameDetailFocus(sim.rides[castle].slot);SendMessageW(window,WM_MOUSEWHEEL,MAKEWPARAM(0,120),0);require(rideCamera.active,"Wheel cancelled locked camera move");for(int i=0;i<20;i++)gameTick(.05);float lockedYaw=yaw,lockedPitch=pitch,lockedZoom=zoom,lockedPan=panX;SendMessageW(window,WM_RBUTTONDOWN,0,MAKELPARAM(600,400));SendMessageW(window,WM_MOUSEMOVE,MK_RBUTTON,MAKELPARAM(700,480));SendMessageW(window,WM_RBUTTONUP,0,0);SendMessageW(window,WM_MOUSEWHEEL,MAKEWPARAM(0,120),0);SendMessageW(window,WM_KEYDOWN,'Q',0);require(yaw==lockedYaw&&pitch==lockedPitch&&zoom==lockedZoom&&panX==lockedPan&&std::abs(pitch-48)<.01,"Detail camera not locked at 42-degree downward angle");sim.paused=pauseBefore;
    overviewCamera.valid=false;rideDetails=false;gameFocus(sim.rides[castle].slot);yaw=27;panX=.03f;panY=.02f;gameClampCamera();
    const auto overviewCenter=sceneCenter;const float overviewYaw=yaw,overviewZoom=zoom,overviewRadius=sceneRadius,overviewPanX=panX,overviewPanY=panY;
    rideDetails=true;gameDetailFocus(sim.rides[castle].slot);for(int i=0;i<20;i++)gameTick(.05);
    int oldSelection=selectedRide;gameAction(100);gameAction(20);require(selectedRide==oldSelection&&rideDetails,"Detail selection escaped modal mode");
    auto oldButtons=gameButtons;gameButtons.clear();gameClick(5,5);require(selectedRide==oldSelection&&rideDetails&&!shopOpen,"World click escaped detail mode");gameButtons=oldButtons;
    int otherRide=castle==0?1:0;gameFocus(sim.rides[otherRide].slot);gameButtons.clear();require(gamePickSlot(width/2,height/2)==sim.rides[otherRide].slot,"Other ride click test target missed");gameClick(width/2,height/2);require(selectedRide==oldSelection&&rideDetails,"Another ride could be selected in details");gameButtons=oldButtons;
    gameAction(40);require(!rideDetails&&!rideCamera.active,"Close did not restore overview");for(int i=0;i<20;i++)gameTick(.05);
    require(sceneCenter==overviewCenter&&std::abs(yaw-overviewYaw)<.001&&std::abs(zoom-overviewZoom)<.001&&std::abs(sceneRadius-overviewRadius)<.001&&std::abs(panX-overviewPanX)<.001&&std::abs(panY-overviewPanY)<.001,"Overview view was not restored");
    rideDetails=true;gameDetailFocus(sim.rides[castle].slot);gameTick(.1);gameAction(19);for(int i=0;i<20;i++)gameTick(.05);require(!rideDetails&&sceneCenter==overviewCenter&&std::abs(yaw-overviewYaw)<.001,"Escape during transition failed");
    std::ofstream(out/L"detail_return_test.json")<<"{\"passed\":true,\"overview_restored\":true,\"selection_locked\":true,\"escape_during_entry\":true}";
    int mechanical=0;for(int id:{409,410,430,445,449,463,465,471,467,476,511}){auto&a=gameAssets.at(id);gameMechanicalPose(a,0,false);std::vector<Mat> resting;for(auto&n:a.model.nodes)resting.push_back(n.world);
        gameMechanicalPose(a,.8,true);bool moved=false,passengerMoved=false;for(size_t i=0;i<a.model.nodes.size();i++)if(resting[i]!=a.model.nodes[i].world){moved=true;passengerMoved|=a.model.nodes[i].locator;}require(moved&&passengerMoved&&resting[0]==a.model.nodes[0].world,"Mechanical ride/guest did not move together");
        gameMechanicalPose(a,2,false);for(size_t i=0;i<a.model.nodes.size();i++)require(resting[i]==a.model.nodes[i].world,"Empty mechanical ride moved");mechanical++;
    }
    {auto prior=sim;for(int id:{409,449,465,467,476,511,522,550}){
        sim=prior;sim.lockedSlots.clear();for(auto&r:sim.rides){r.riders.clear();r.queue.clear();r.timer=0;}sim.guests.clear();
        int type=-1,slot=-1;for(size_t i=0;i<sim.types.size();i++)if(sim.types[i].model==id)type=int(i);for(size_t i=0;i<sim.slots.size();i++)if(sim.canBuild(int(i),type)){slot=int(i);break;}
        require(slot>=0,"No mechanical test plot");int ride=sim.build(slot,type);for(int i=0;i<std::min(2,sim.types[type].capacity);i++){playtest::Guest g;g.tile=sim.entrance(slot);g.state=playtest::State::Riding;g.target=ride;g.wait=10;g.seat=i;sim.guests.push_back(g);sim.rides[ride].riders.push_back(i);}sim.rides[ride].timer=sim.rides[ride].riders.empty()?0:10;sim.rides[ride].running=!sim.rides[ride].riders.empty();
        selectedRide=ride;gameFocus(slot);rideDetails=true;sim.rides[ride].animationTime=.4;capture(out/(L"motion-"+std::to_wstring(id)+L"-a.png"));sim.rides[ride].animationTime=1.2;capture(out/(L"motion-"+std::to_wstring(id)+L"-b.png"));sim.validate();
    }sim=prior;selectedRide=castle;gameFocus(sim.rides[castle].slot);rideDetails=false;}
    {auto center=sceneCenter;float oldZoom=zoom;sceneCenter={91*32.f,-144*32.f,0};sceneRadius=600;zoom=gameZoomTarget=5;pitch=gameTilt(zoom);panX=panY=0;capture(out/L"entrance-connected.png");sceneCenter=center;zoom=gameZoomTarget=oldZoom;}
    // Dragging the world must pan, not select a ride on mouse-down.
    rideDetails=false;gameButtons.clear();gameFocus(sim.rides[castle].slot);float oldPan=panX;
    SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(width/2,height/2));SendMessageW(window,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(width/2+40,height/2));SendMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(width/2+40,height/2));require(panX!=oldPan&&!rideDetails,"Left drag selected ride or failed to pan");
    {auto prior=sim;sim.paused=false;sim.speed=1;selectedRide=castle;auto&r=sim.rides[castle];r.open=true;r.challenge=-1;int balance=sim.money;require(sim.tap(castle),"Empty 20s session could not start");require(sim.visualPassengers(r)==sim.types[r.type].capacity,"Session seats not full");for(int i=0;i<420;i++){if(i%11==0)sim.tap(castle);gameTick(.05);}require(r.challenge==-2&&sim.money==balance&&!sim.loot.empty()&&!sim.tap(castle),"20s session did not stop and credit reward");sim.validate();rideDetails=true;gameFocus(r.slot);capture(out/L"session-20s-finished.png");rideDetails=false;sim=prior;}
    {auto prior=sim;std::ofstream report(out/L"requested-rides-animation.json");report<<"[";bool first=true;
    for(int id:{460,425,461,520,418,522,420,427,521,452,453}){
        sim=prior;sim.lockedSlots.clear();sim.rides.clear();sim.guests.clear();int type=-1,slot=-1;for(size_t i=0;i<sim.types.size();i++)if(sim.types[i].model==id)type=int(i);for(size_t i=0;i<sim.slots.size();i++)if(sim.canBuild(int(i),type)){slot=int(i);break;}require(slot>=0,"Requested ride has no plot");int ride=sim.build(slot,type);require(sim.tap(ride),"Requested ride session failed");auto&a=gameAssets.at(id);require(!a.animation.tracks.empty(),"Requested ride lacks operating clip");gamePose(a,0);std::vector<Mat> initial;for(const auto&n:a.model.nodes)initial.push_back(n.world);int cycleFrames=1;for(const auto&track:a.animation.tracks)cycleFrames=std::max(cycleFrames,track.frames);for(int frame=0;frame<24;frame++){double duration=cycleFrames*a.animation.frameMS;gamePose(a,duration*frame/24);require(initial[0]==a.model.nodes[0].world,"Operating clip moved root during cycle");for(const auto&node:a.model.nodes)for(float value:node.world)require(std::isfinite(value),"Nonfinite operating pose");}gamePose(a,6000);int moved=0,seatsMoved=0;for(size_t n=0;n<a.model.nodes.size();n++)if(initial[n]!=a.model.nodes[n].world){moved++;if(a.model.nodes[n].locator)seatsMoved++;}require(moved>0&&initial[0]==a.model.nodes[0].world,"Operating animation moves terrain or is static");
        selectedRide=ride;gameFocus(slot);sceneRadius=600;zoom=gameZoomTarget=std::clamp(26.f/std::max(sim.slots[slot].w,sim.slots[slot].h),1.2f,7.f);pitch=gameTilt(zoom);yaw=-35;rideDetails=true;
        for(int sec:{0,6,14}){sim.rides[ride].animationTime=sec;capture(out/(L"audit-"+std::to_wstring(id)+L"-"+std::to_wstring(sec)+L".png"));}
        if(!first)report<<",";first=false;report<<"{\"model\":"<<id<<",\"group\":"<<a.animation.group<<",\"moving_nodes\":"<<moved<<",\"moving_seats\":"<<seatsMoved<<",\"passengers\":"<<sim.visualPassengers(sim.rides[ride])<<"}";sim.validate();double motionBefore=sim.rides[ride].animationTime;sim.cancelChallenge(ride);sim.guests.clear();sim.tick(.05);require(sim.rides[ride].animationTime==motionBefore,"Empty requested ride advanced animation");gamePose(a,0);for(size_t n=0;n<initial.size();n++)require(initial[n]==a.model.nodes[n].world,"Requested ride rest pose changed");
    }report<<"]";sim=prior;selectedRide=castle;rideDetails=false;gameFocus(sim.rides[castle].slot);}
    {auto prior=sim;auto priorSelected=selectedRide;rideDetails=false;shopOpen=false;territoryPanel=-1;sim.openTerritories.insert(18);
        require(!sim.setTheme(17,3)&&sim.themes[17]==0,"Entrance theme is not fixed");const auto&p=themePlots.at(18);sceneCenter={(p.x+p.w/2.f)*32,-(p.y+p.h/2.f)*32,0};sceneRadius=600;zoom=gameZoomTarget=4;pitch=45;yaw=-35;panX=panY=0;
        auto hit=uiWorld(sceneCenter[0],sceneCenter[1],0);gameButtons.clear();gameClick(hit.first,hit.second);require(themePanel==18,"Theme plot not clickable");for(int n=0;n<20;n++)gameTick(.05);capture(out/L"territory-theme-menu.png");
        for(int t=0;t<12;t++){themePanel=18;gameAction(60+t);gameAction(72);require(sim.themes[18]==t&&themePanel<0,"Theme apply failed");for(int other=0;other<20;other++)if(other!=18)require(sim.themes[other]==prior.themes[other],"Theme escaped territory");
            std::ostringstream saved;sim.write(saved);auto copy=sim;std::istringstream input(saved.str());copy.read(input);require(copy.themes==sim.themes,"Theme save roundtrip failed");
            sceneCenter={127*32.f,-127*32.f,0};sceneRadius=600;zoom=gameZoomTarget=1.6f;pitch=50;capture(out/(L"territory-theme-"+std::to_wstring(t)+L".png"));}
        sim=prior;selectedRide=priorSelected;themePanel=-1;sceneCenter={109*32.f,-143*32.f,0};sceneRadius=600;zoom=gameZoomTarget=3;pitch=50;capture(out/L"park-perimeter-joined.png");
        std::ofstream(out/L"territory-theme-test.json")<<"{\"passed\":true,\"themes\":12,\"reserved_plots\":20,\"local_only\":true,\"save_roundtrip\":true}";
        gameFocus(sim.rides[selectedRide].slot);
    }
    {auto prior=sim;sim.money=1000000;sim.lockedSlots.clear();sim.guests.clear();sim.rides.clear();rideDetails=false;themePanel=-1;shopOpen=false;
        for(int size:{4,6,16}){int slot=-1,type=-1;for(size_t i=0;i<sim.types.size();i++)if(sim.types[i].w==size&&sim.types[i].h==size){type=int(i);break;}for(size_t i=0;i<sim.slots.size();i++)if(sim.canBuild(int(i),type)){slot=int(i);break;}require(slot>=0,"Missing construction test plot");int index=sim.purchase(slot,type);require(index>=0&&!sim.tap(index),"Unfinished building accepts player");selectedRide=index;gameFocus(slot);yaw=-35;pitch=35;sim.rides[index].buildRemaining=15;capture(out/(L"construction-"+std::to_wstring(size)+L"-early.png"));sim.rides[index].buildRemaining=3;capture(out/(L"construction-"+std::to_wstring(size)+L"-late.png"));sim.tick(3);require(sim.rides[index].buildRemaining==0,"Construction did not finish");}
        sim=prior;selectedRide=castle;rideDetails=false;gameFocus(sim.rides[castle].slot);for(auto&bin:sim.bins)if(bin.territory==17)bin.remaining=0;capture(out/L"trash-clean-icons.png");int index=-1;for(size_t i=0;i<sim.bins.size();i++)if(sim.bins[i].territory==17){index=int(i);break;}require(index>=0,"Entrance territory has no original trash bins");int flags=sim.flags;gameAction(3000+index);while(!sim.loot.empty())gameAction(4000);require(sim.flags==flags+2&&sim.bins[index].remaining>=60,"Trash clean action failed");gameAction(3000+index);while(!sim.loot.empty())gameAction(4000);require(sim.flags==flags+2,"Trash double reward");sim=prior;selectedRide=castle;
        std::ofstream(out/L"session-construction-trash-test.json")<<"{\"passed\":true,\"session_seconds\":20,\"maximum_boarding_seconds\":60,\"construction_sizes\":[4,6,16],\"trash_bins\":"<<sim.bins.size()<<",\"detail_camera_locked\":true}";
    }
    sfx.stop();sfx.silent=true;sfx.enabled=true;sfx.focused=true;int audioEffects=0;
    for(const char*event:{"click","select","build","collect","sell","unlock"}){sfx.lastEvent.clear();sfx.lastTap=0;require(sfx.play(root/L"extracted/runtime/audio/sfx",event),"SFX playback failed");audioEffects++;sfx.stop();}
    sfx.enabled=false;require(!sfx.play(root/L"extracted/runtime/audio/sfx","click"),"Muted SFX played");sfx.enabled=true;
    settingsOpen=true;capture(out/L"settings-sfx.png");settingsOpen=false;
    std::ofstream(out/L"motion_camera_sfx_test.json")<<"{\"passed\":true,\"mechanical_rides\":"<<mechanical<<",\"original_sfx_played_silently\":"<<audioEffects<<",\"camera_front_transition\":true,\"manual_cancel\":true}";
    {auto prior=sim;bool priorDay=dayNightEnabled;dayNightEnabled=true;rideDetails=false;shopOpen=false;selectedRide=castle;gameFocus(sim.rides[castle].slot);
        sim.time=0;capture(out/L"lighting-day.png");require(gameDaylight()>.99,"Daylight phase failed");sim.time=180;require(gameDaylight()<.01,"Night phase failed");capture(out/L"lighting-night.png");gameAction(74);require(gameDaylight()==1,"Daylight toggle failed");dayNightEnabled=true;
        std::ofstream prices(out/L"local-balance.tsv");prices<<"model\tname\tsize\tcash\tbuild_seconds\n";for(size_t i=0;i<sim.types.size();i++){const auto&t=sim.types[i];prices<<t.model<<'\t'<<t.name<<'\t'<<t.w<<'\t'<<sim.buildPrice(int(i))<<'\t'<<sim.buildSeconds(int(i))<<'\n';}
        std::ofstream audit(out/L"operating-coverage.tsv");audit<<"model\tgroup\tmoving_nodes\tmoving_seats\n";int tested=0;
        for(size_t type=0;type<sim.types.size();type++){auto&a=gameAssets.at(sim.types[type].model);if(typeCategories[type]||a.animation.tracks.empty())continue;
            gamePose(a,0);std::vector<Mat> start;for(const auto&n:a.model.nodes)start.push_back(n.world);std::set<int> changed;for(int frame=1;frame<=12;frame++){gamePose(a,frame*371.);require(start[0]==a.model.nodes[0].world,"Operating catalog moved terrain root");for(size_t n=0;n<a.model.nodes.size();n++){for(float x:a.model.nodes[n].world)require(std::isfinite(x),"Invalid operating matrix");if(start[n]!=a.model.nodes[n].world)changed.insert(int(n));}}
            require(!changed.empty(),"Operating catalog contains static clip");int seats=0;for(int n:changed)if(a.model.nodes[n].locator)seats++;audit<<a.model.id<<'\t'<<a.animation.group<<'\t'<<changed.size()<<'\t'<<seats<<'\n';tested++;
        }
        require(tested>=50,"Operating coverage unexpectedly small");
        sim.lockedSlots.clear();sim.enforceTerritories=false;sim.rides.clear();sim.guests.clear();dayNightEnabled=false;
        for(int id:{416,424,449,463,476,515,425}){int type=-1,slot=-1;for(size_t i=0;i<sim.types.size();i++)if(sim.types[i].model==id)type=int(i);for(size_t i=0;i<sim.slots.size();i++)if(sim.canBuild(int(i),type)){slot=int(i);break;}require(slot>=0,"New animation capture has no plot");sim.rides.clear();int ride=sim.build(slot,type);selectedRide=ride;rideDetails=true;gameFocus(slot);yaw=-35;pitch=35;sim.tap(ride);sim.rides[ride].animationTime=3.1;capture(out/(L"expanded-motion-"+std::to_wstring(id)+L".png"));
            if(id==425){sim.cancelChallenge(ride);sim.time=0;capture(out/L"water-idle-a.png");sim.time=1.4;capture(out/L"water-idle-b.png");}}
        auto&water=gameAssets.at(425);require(!water.idle.tracks.empty()&&water.idle.group==47&&water.animation.group==46,"Idle/operating clips overwritten");gamePoseClip(water,water.idle,0);auto initial=water.model.nodes;gamePoseClip(water,water.idle,1777);bool moved=false;for(size_t n=0;n<initial.size();n++)moved|=initial[n].world!=water.model.nodes[n].world;require(moved,"Water idle is frozen");
        sim=prior;sim.paused=false;sim.speed=1;rideDetails=true;selectedRide=castle;auto&r=sim.rides[castle];r.challenge=-1;r.open=true;require(sim.tap(castle),"Cheer session failed");r.challenge=16.4;int before=cheerCues;accumulator=0;gameTick(.2);require(cheerCues==before+1,"Late cheer missing");gameTick(.2);require(cheerCues==before+1,"Late cheer repeated");
        sim=prior;rideDetails=false;dayNightEnabled=priorDay;selectedRide=castle;std::ofstream(out/L"motion-balance-light-test.json")<<"{\"passed\":true,\"operating_rides_tested\":"<<tested<<",\"water_idle\":true,\"cheer_once_at_seconds\":16.5,\"day_night_seconds\":360}";
    }
    {auto prior=sim;sim.money=1000000;sim.lockedSlots.clear();sim.rides.clear();sim.guests.clear();selectedRide=-1;rideDetails=false;shopSlot=-1;shopOpen=true;shopCategory=2;capture(out/L"staff-shop.png");shopOpen=false;
        int slot=-1;for(size_t i=0;i<sim.slots.size();i++)if(sim.territory(int(i))==17&&sim.buyStaff(int(i))){slot=int(i);break;}require(slot>=0,"No staff house plot");gameFocus(slot);for(auto&b:sim.bins)if(b.territory==17)b.remaining=0;int flags=sim.flags;for(int n=0;n<2400;n++)sim.tick(.05);require(sim.flags>flags,"Staff did not clean bins");sim.validate();capture(out/L"staff-patrol.png");sim.staff[0].tile=sim.entrance(slot);sim.staff[0].route.clear();sim.staff[0].step=0;sim.staff[0].progress=0;zoom=gameZoomTarget=7;for(int kind=0;kind<5;kind++){sim.staff[0].kind=kind;capture(out/(L"mascot-house-"+std::to_wstring(kind)+L".png"));}sim.staff[0].kind=1;
        sceneCenter={91*32.f,-132*32.f,0};sceneRadius=600;zoom=gameZoomTarget=4;yaw=-35;pitch=35;for(int t=0;t<3;t++){sim.officeTheme=t;sim.officeWings=7;capture(out/(L"office-"+std::to_wstring(t)+L".png"));}gameAction(76);for(int n=0;n<20;n++)gameTick(.05);capture(out/L"office-menu.png");gameAction(77);
        shopSlot=-1;for(size_t i=0;i<sim.slots.size();i++)if(sim.slots[i].w==16){shopSlot=int(i);break;}shopOpen=true;shopCategory=1;capture(out/L"large-rides-only.png");require(shopCategory==0,"Large plot retained shop category");for(const auto&button:gameButtons)require(button.action!=32&&button.action!=78,"Large plot shows shops/staff");shopOpen=false;shopSlot=-1;sim=prior;selectedRide=castle;gameFocus(sim.rides[castle].slot);sim.time=180;capture(out/L"night-local-lights.png");sim=prior;std::ofstream(out/L"facilities-test.json")<<"{\"passed\":true,\"staff_cleaning\":true,\"office_themes\":3,\"large_rides_only\":true}";}
    {require(LoadIconW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(1))!=nullptr,"Embedded app icon missing");startupActive=true;startupReady=false;startupProgress=.4f;startupMessage="Loading your park";capture(out/L"startup-loading.png");startupReady=true;startupProgress=1;capture(out/L"startup-menu.png");double stopped=sim.time;SendMessageW(window,WM_TIMER,1,0);require(sim.time==stopped,"Simulation ran behind startup menu");SendMessageW(window,WM_KEYDOWN,VK_RETURN,0);require(!startupActive,"Enter park failed");}
    {auto prior=sim;sim.money=10000;sim.rides.clear();sim.guests.clear();sim.staff.clear();sim.lockedSlots.clear();
        int house=-1;for(size_t i=0;i<sim.slots.size();i++)if(sim.buyStaff(int(i))){house=int(i);break;}
        require(house>=0,"Staff detail fixture failed");gameFocus(house);gameButtons.clear();gameClick(width/2,height/2);
        require(rideDetails&&staffDetail==house,"House did not open details");capture(out/L"staff-detail.png");
        int cash=sim.money;gameAction(95);require(!rideDetails&&sim.staffAt(house)<0&&sim.money==cash+900,"Staff sale failed");
        gameAction(44);gameAction(96);gameAction(98);require(sim.money==cash+900,"New game cancelled incorrectly");
        gameAction(96);gameAction(97);require(sim.money==startingPark.money&&sim.staff.empty()&&sim.time==startingPark.time,"New game failed");
        sim=prior;selectedRide=castle;gameFocus(sim.rides[castle].slot);
    }
    {auto prior=sim;auto camera=currentShot();sim=startingPark;require(sim.openTerritories==std::set<int>{17},"New game opened extra land");
        for(const auto&r:sim.rides)require(sim.territory(r.slot)==17,"New game facility outside starting tile");
        parade={};shopOpen=true;sim.addFlags(sim.rankTarget());gameTick(.05);
        require(parade.path.empty(),"Parade interrupted shop");gameSave();gameLoad();shopOpen=false;gameTick(.05);require(parade.follow&&parade.path.front()==sim.spawn,"Parade did not enter through gate");
        for(int tile:parade.path)require(sim.openTerritories.count(sim.tileTerritory(tile)),"Parade crossed a locked territory");
        for(int n=0;n<80;n++)advanceParade(.05);capture(out/L"rank-parade.png");
        for(int n=0;n<25;n++)advanceParade(.05);require(!parade.follow&&!parade.path.empty(),"Parade camera did not release");
        for(int n=0;n<1201;n++)advanceParade(.05);require(parade.path.empty(),"Parade did not finish");
        sim=prior;parade={};blendShot(camera,camera,1);
    }
    {auto prior=sim;sim.lockedSlots.clear();sim.enforceTerritories=false;sim.rides.clear();sim.guests.clear();int verified=0;
        for(size_t type=0;type<sim.types.size();type++){
            auto&t=sim.types[type];if(!restoredRides.count(t.model))continue;shopSlot=-1;require(shopFits(int(type)),"Restored attraction missing from shop");
            int slot=-1;for(size_t i=0;i<sim.slots.size();i++)if(sim.canBuild(int(i),int(type))){slot=int(i);break;}
            require(slot>=0&&sim.build(slot,int(type))==0&&sim.tap(0),"Restored ride cannot build or play");
            auto&a=gameAssets.at(t.model);sim.rides[0].animationTime=0;gameDrawRide(int(type),slot,0);
            std::vector<Mat> initial;for(const auto&n:a.model.nodes)initial.push_back(n.world);
            bool moved=false;for(int step=1;step<=8&&!moved;step++){
                sim.rides[0].animationTime=step*.371;gameDrawRide(int(type),slot,0);
                for(size_t n=1;n<initial.size();n++)moved|=initial[n]!=a.model.nodes[n].world;
            }
            require(moved,"Restored ride did not animate during play");++verified;sim.rides.clear();
        }
        require(verified==39,"Unexpected restored attraction count");sim=prior;
        std::ofstream(out/L"restored-rides.json")<<"{\"shop_build_play_verified\":"<<verified<<"}";
    }
    {std::vector<double> timings;LARGE_INTEGER frequency,start,end;QueryPerformanceFrequency(&frequency);for(int i=0;i<35;i++){QueryPerformanceCounter(&start);render(false);glFinish();QueryPerformanceCounter(&end);if(i>=5)timings.push_back((end.QuadPart-start.QuadPart)*1000./frequency.QuadPart);}std::sort(timings.begin(),timings.end());std::ofstream(out/L"render_timing.json")<<"{\"samples\":30,\"median_ms\":"<<timings[15]<<",\"p95_ms\":"<<timings[28]<<",\"presentation_included\":false}";}
    int shown=0;shopSlot=-1;for(size_t i=0;i<sim.types.size();i++){require(shopFits(int(i))==(typeThumbs[i]!=0||!sim.types[i].shop),"Artwork filter failed");if(shopFits(int(i)))require(uiBounds.count(typeThumbs[i])&&uiAssets.count(typeThumbs[i])&&fs::is_regular_file(assetPath(uiAssets.at(typeThumbs[i]))),"Shop artwork missing");shown+=shopFits(int(i));}
    shopOpen=true;
    for(int id:{531,576,414}){
        auto found=std::find_if(sim.types.begin(),sim.types.end(),[&](const auto&t){return t.model==id;});
        int index=int(found-sim.types.begin());shopCategory=typeCategories[index];
        for(shopPage=0;shopPage<100;shopPage++){shopSelection=index;render(false);if(shopSelection==index)break;}
        require(shopSelection==index,"Shop selection unreachable");
        capture(out/("shop-reviewed-"+std::to_string(id)+".png"));
    }
    shopOpen=false;
    std::ofstream(out/L"territory_test.json")<<"{\"passed\":true,\"shop_with_artwork\":"<<shown<<",\"camera_clamped\":true,\"ride_hud_isolated\":true}";
    std::ofstream(out/L"economy_test.json")<<"{\"passed\":true,\"collect_money_flags\":true,\"minigame_20_seconds\":true,\"tap_acceleration\":true,\"cash_ticket_upgrade\":true,\"sell_releases_guests\":true,\"save_resume_economy\":true}";
    std::ofstream(out/L"expansion_test.json")<<"{\"passed\":true,\"catalog_placeable\":"<<matchingCatalog<<",\"decorations\":"<<parkDecorations.size()<<",\"original_ui_shop_placement\":true,\"gate_upgrade_save\":true,\"bus_guests\":true}";
    std::ofstream report(out/L"playtest_test.json");report<<"{\"passed\":true,\"guests\":"<<sim.guests.size()<<",\"rides\":"<<sim.rides.size()<<",\"completed_visits\":"<<sim.served<<",\"simulated_seconds\":"<<sim.time<<",\"ui_placement\":true,\"pause_speed\":true,\"save_restore\":true,\"close_reopen\":true}";
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR args,int show){
    ULONG_PTR token=0;Gdiplus::GdiplusStartupInput startup;Gdiplus::GdiplusStartup(&token,&startup,nullptr);int code=0;
    try{wchar_t exe[32768];DWORD n=GetModuleFileNameW(nullptr,exe,32768);require(n&&n<32768,"Executable path failed");root=fs::path(exe).parent_path().parent_path();
        testing=std::wstring(args).find(L"--self-test")!=std::wstring::npos;game=fs::path(exe).filename().wstring().rfind(L"ThemeParkGame",0)==0||std::wstring(args).find(L"--play")!=std::wstring::npos;
        WNDCLASSW wc{};wc.style=CS_OWNDC;wc.lpfnWndProc=proc;wc.hInstance=instance;wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(1));wc.lpszClassName=L"ThemeParkOriginal3D";require(RegisterClassW(&wc)!=0,"Window class failed");
        window=CreateWindowExW(0,wc.lpszClassName,L"Theme Park | Original 3D",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1280,840,nullptr,nullptr,instance,nullptr);require(window!=nullptr,"Window failed");initGL();
        if(game&&!testing){startupActive=true;ShowWindow(window,show);startupRender();}loadCatalog();if(startupActive){startupProgress=.2f;startupMessage="Loading original models and textures";startupRender();}
        selectModel(425<int(catalog.size())?425:0);
        if(game){initGame();if(startupActive){startupReady=true;startupProgress=1;startupRender();}}else if(!testing&&std::wstring(args).find(L"--models")==std::wstring::npos&&fs::is_regular_file(root/L"extracted/runtime/park_scene.tsv"))setPark(true);
        if(std::wstring(args).find(L"--shop-art")!=std::wstring::npos)exportShopArt();
        else if(std::wstring(args).find(L"--facility-capture")!=std::wstring::npos){for(int id:{28,29,30,31,32,33,34,40,41,42,45,59,73,87,101,115,129,143,157,171,185,199,207}){selectModel(id);capture(root/L"analysis/runtime_test"/(L"facility-"+std::to_wstring(id)+L".png"));}}
        else if(std::wstring(args).find(L"--ride-sheet")!=std::wstring::npos){for(int id=200;id<730;id++){selectModel(id);capture(root/L"analysis/runtime_test"/(L"catalog-"+std::to_wstring(id)+L".png"));}}
        else if(std::wstring(args).find(L"--catalog-capture")!=std::wstring::npos){for(int id:{0,7,425,427,515,522}){selectModel(id);capture(root/L"analysis/runtime_test"/(L"catalog-"+std::to_wstring(id)+L".png"));}}
        else if(testing){if(game)verifyGame();else verify();}else{lastTick=GetTickCount64();SetTimer(window,1,16,nullptr);ShowWindow(window,show);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}code=int(msg.wParam);}
    }catch(const std::exception&e){fs::create_directories(root/L"analysis/runtime_test");std::ofstream(root/L"analysis/runtime_test/model_error.txt")<<e.what();if(!testing)MessageBoxA(nullptr,e.what(),"Theme Park 3D",MB_ICONERROR);code=1;}
    music.close();sfx.stop();if(context){for(auto&entry:geometryLists)glDeleteLists(entry.second,1);clearUIText();clearSceneMeshes();for(auto&kv:textures)glDeleteTextures(1,&kv.second);if(fontBase)glDeleteLists(fontBase,256);if(gameFont)glDeleteLists(gameFont,256);wglMakeCurrent(nullptr,nullptr);wglDeleteContext(context);}if(dc)ReleaseDC(window,dc);if(IsWindow(window))DestroyWindow(window);Gdiplus::GdiplusShutdown(token);return code;
}

