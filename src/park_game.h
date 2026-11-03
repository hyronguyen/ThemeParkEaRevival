// Included after the native renderer's asset helpers. UI/rules are a local playtest.
#include "park_simulation.h"
#include "park_audio.h"
static playtest::Park sim, startingPark;
static int staffDetail=-1;
static bool confirmRestart=false;
static void gameSound(const std::string&event){if(event=="tap"||event=="board"||event=="complete")return;sfx.silent=testing;sfx.play(root/L"extracted/runtime/audio/sfx",event);}
struct GameAsset{Model model;Animation animation,idle;float minZ=0,spanX=1,spanY=1,centerX=0,centerY=0;};
static std::map<int,GameAsset> gameAssets;
static std::map<int,std::vector<int>> passengerPoses;
// Pose selection fitted against authored ride seats; exact original AnimSet links
// remain separately recorded in passengerPoses (cars/karts).
static const std::map<int,int> fittedPassengerPose={{409,411},{410,411},{416,417},{418,417},{420,417},{422,417},{423,417},{424,417},{425,426},{427,523},{428,417},{429,411},{430,411},{431,435},{433,435},{434,435},{436,437},{445,468},{447,448},{449,411},{458,459},{460,462},{461,462},{463,464},{465,466},{467,468},{470,411},{471,411},{472,411},{476,411},{479,411},{490,411},{491,411},{500,501},{509,510},{511,411},{512,411},{515,516},{517,518},{520,411},{522,523}};
static int gamePassengerModel(int model){auto it=fittedPassengerPose.find(model);return it==fittedPassengerPose.end()?0:it->second;}

static int buildType=-1,selectedRide=-1,hoverSlot=-1,buildRotation=0;
static int selectedGuest=-1;
static bool debugHUD=false,shopOpen=false,rideDetails=false,settingsOpen=false;
static bool staffPlacement=false,officePanel=false;
static int staffKind=1;
static const int staffArtwork[]={42,26,30,34,38};
static const int mascotBases[]={33,36,39,42,45};
static const char*staffNames[]={"RHINO HOUSE","CHICKEN HOUSE","SHARK HOUSE","BEAR HOUSE","STRONGMAN HOUSE"};
struct RewardBurst{float x,y;int cash,flags;double age=0;};
static std::vector<RewardBurst> rewardBursts;
static int shopCategory=0,shopPage=0,shopSelection=4,shopSlot=-1;
static float gameZoomTarget=4;
static float gameTilt(float z){float t=std::clamp((z-1.2f)/5.8f,0.f,1.f);return 22+36*t*t*(3-2*t);}
static bool shopFits(int type);
static bool plotFits(int type){if(shopSlot<0)return true;const auto&s=sim.slots[shopSlot];const auto&t=sim.types[type];if(s.w==16&&s.h==16&&t.shop)return false;return (t.w==s.w&&t.h==s.h)||(t.h==s.w&&t.w==s.h);}
static const std::set<int> restoredRides={419,473,475,484,486,487,488,489,490,491,492,493,495,497,498,499,500,502,503,508,509,511,512,513,514,529,531,533,534,535,536,537,538,539,540,541,542,543,544};
static std::vector<int> typeThumbs,typeCategories;
static bool shopFits(int type){return type>=0&&type<int(typeThumbs.size())&&(typeThumbs[type]!=0||!sim.types[type].shop)&&plotFits(type);}
static int territoryPanel=-1,themePanel=-1,themeSelection=0;
struct TerritoryTheme{int statue=0,ground=392,fence4=669,fence6=670,fence16=671,gate=216;std::string path,name;};
static std::vector<TerritoryTheme> territoryThemes;
static std::map<int,playtest::Slot> themePlots;
static int territoryAt(float x,float y){return std::clamp(int(std::floor((-y/32-1)/36)),0,3)*5+std::clamp(int(std::floor((x/32-1)/36)),0,4);}
static bool gameVisible(float x,float y,float radius=100,float z=0){float a=yaw*.01745329252f,p=pitch*.01745329252f,scale=height*zoom/(2.7f*sceneRadius);x-=sceneCenter[0];y-=sceneCenter[1];float sx=width*.5f+(x*std::cos(a)-y*std::sin(a)+panX*sceneRadius)*scale,sy=height*.5f-((x*std::sin(a)+y*std::cos(a))*std::cos(p)+(z-sceneCenter[2])*std::sin(p)+panY*sceneRadius)*scale,margin=radius*scale+40;return sx>=-margin&&sx<=width+margin&&sy>=-margin&&sy<=height+margin;}
static void gameSceneItem(const SceneInstance&i){
    int theme=sim.themes[territoryAt(i.x,i.y)];const auto&t=territoryThemes.at(theme);
    if(i.kind=='G'){drawParts(gameAssets.at(t.ground).model);return;}
    if(i.kind=='P'&&theme){glCallList(sceneMeshes.at({i.model,t.path}).list);return;}
    glCallList(sceneMeshes.at({i.model,i.texture}).list);
}
static std::map<int,std::string> uiAssets;
static std::map<int,std::array<float,4>> uiBounds;
struct ParkDecoration{int model;float x,y;};
static std::vector<ParkDecoration> parkDecorations;

static double accumulator=0;
static bool dayNightEnabled=true;
static int cheerCues=0;
static float gameDaylight(){return dayNightEnabled?float(.5+.5*std::cos(sim.time*6.283185307/360.)):1.f;}
static void gameLighting(){float day=gameDaylight(),sun=dayNightEnabled?float(sim.time*6.283185307/360.):0.f;
    GLfloat ambient[]={.34f+.4f*day,.38f+.36f*day,.52f+.22f*day,1};
    GLfloat diffuse[]={.08f+.22f*day,.07f+.20f*day,.10f+.16f*day,1};
    GLfloat direction[]={float(std::cos(sun)),float(std::sin(sun)),.85f,0};
    glEnable(GL_LIGHTING);glEnable(GL_LIGHT0);glEnable(GL_NORMALIZE);glEnable(GL_COLOR_MATERIAL);glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);glLightfv(GL_LIGHT0,GL_POSITION,direction);glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
    float cameraAngle=yaw*3.14159265f/180,cameraPitch=pitch*3.14159265f/180;float lightCenterX=sceneCenter[0]-sceneRadius*(panX*std::cos(cameraAngle)+panY/std::cos(cameraPitch)*std::sin(cameraAngle)),lightCenterY=sceneCenter[1]+sceneRadius*(panX*std::sin(cameraAngle)-panY/std::cos(cameraPitch)*std::cos(cameraAngle));
    std::vector<std::pair<float,int>> lamps;for(size_t i=0;i<sim.rides.size();i++)if(sim.rides[i].buildRemaining<=0){const auto&s=sim.slots[sim.rides[i].slot];float x=(s.x+s.w/2.f)*32,y=-(s.y+s.h/2.f)*32;lamps.push_back({std::hypot(x-lightCenterX,y-lightCenterY),int(i)});}std::sort(lamps.begin(),lamps.end());
    float night=1-day;for(int n=0;n<7;n++){GLenum light=GL_LIGHT1+n;glDisable(light);if(n>=int(lamps.size())||night<.02f)continue;const auto&s=sim.slots[sim.rides[lamps[n].second].slot];float range=std::max(100.f,s.w*24.f);GLfloat pos[]={(s.x+s.w/2.f)*32,-(s.y+s.h/2.f)*32,42,1},color[]={night*.85f,night*.57f,night*.26f,1};glEnable(light);glLightfv(light,GL_POSITION,pos);glLightfv(light,GL_DIFFUSE,color);glLightf(light,GL_CONSTANT_ATTENUATION,1);glLightf(light,GL_LINEAR_ATTENUATION,0);glLightf(light,GL_QUADRATIC_ATTENUATION,2*sceneRadius*sceneRadius/(range*range));}
}
static std::string gameNotice="Choose a ride, then click a green plot. Right-drag moves the camera.";
struct GameButton{int x,y,w,h,action;};
static std::vector<GameButton> gameButtons;
static GLuint gameFont=0;
struct RideCameraMove {bool active=false;double elapsed=0;std::array<float,3> from{},to{};float yawFrom=0,yawTo=0,zoomFrom=0,zoomTo=0,pitchFrom=0,panXFrom=0,panYFrom=0,radiusFrom=0,panXTo=0,panYTo=0,radiusTo=600,pitchTo=0;} rideCamera;
struct OverviewCamera {bool valid=false;std::array<float,3> center{};float yaw=0,pitch=0,zoom=0,radius=0,panX=0,panY=0;} overviewCamera;
static bool inspectionOpen(){return rideDetails||officePanel||themePanel>=0;}
static void focusBuilding(const playtest::Slot&s,bool sidePanel){
    if(!overviewCamera.valid)overviewCamera={true,sceneCenter,yaw,pitch,zoom,sceneRadius,panX,panY};
    rideCamera={};rideCamera.active=true;rideCamera.elapsed=0;rideCamera.from=sceneCenter;rideCamera.to={float((s.x+s.w/2.)*32),float(-(s.y+s.h/2.)*32),0};
    rideCamera.yawFrom=yaw;rideCamera.yawTo=yaw+std::remainder(-35-yaw,360.f);rideCamera.zoomFrom=zoom;rideCamera.zoomTo=std::clamp(24.f/std::max(s.w,s.h),1.2f,6.f);rideCamera.pitchFrom=pitch;rideCamera.panXFrom=panX;rideCamera.panYFrom=panY;rideCamera.radiusFrom=sceneRadius;rideCamera.pitchTo=48;rideCamera.radiusTo=500;rideCamera.panXTo=sidePanel?-.43f/rideCamera.zoomTo:0;rideCamera.to[2]=sidePanel?32:20;selectedGuest=-1;
}
static void gameDetailFocus(int slot){focusBuilding(sim.slots.at(slot),false);}
static void gameExitDetail(){
    if(rideDetails&&selectedRide>=0&&selectedRide<int(sim.rides.size()))sim.cancelChallenge(selectedRide);
    rideDetails=false;staffDetail=-1;rideCamera.active=false;selectedGuest=-1;
    if(!overviewCamera.valid)return;
    const auto o=overviewCamera;sceneCenter=o.center;yaw=o.yaw;pitch=o.pitch;zoom=gameZoomTarget=o.zoom;sceneRadius=o.radius;panX=o.panX;panY=o.panY;overviewCamera.valid=false;
}
static void gameFocus(int slot){rideCamera.active=false;selectedGuest=-1;const auto&s=sim.slots.at(slot);sceneCenter={float((s.x+s.w/2.)*32),float(-(s.y+s.h/2.)*32),0};sceneRadius=900;zoom=std::clamp(32.f/std::max(s.w,s.h),4.f,7.f);gameZoomTarget=zoom;pitch=gameTilt(zoom);panX=panY=0;}
#include "park_cinematics.h"
static fs::path gameSavePath(){
    if(testing)return root/L"analysis/runtime_test/playtest-save.txt";
    wchar_t local[32768];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",local,32768);require(n>0&&n<32768,"LOCALAPPDATA unavailable");
    return fs::path(local)/L"ThemeParkPreservation/playtest-v7.txt";
}
static void gameSave(){
    auto path=gameSavePath();fs::create_directories(path.parent_path());auto temp=path;temp+=L".tmp";
    {std::ofstream out(temp);require(bool(out),"Cannot create save");sim.write(out);out.flush();require(bool(out),"Save write failed");}
    require(MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"Save replace failed");gameNotice="Park saved on this PC.";
}
static void gameLoad(){std::ifstream in(gameSavePath());require(bool(in),"No saved park yet");sim.read(in);parade={};readyMarkers.clear();gateFilm={};overviewCamera.valid=false;rideDetails=false;sim.spawn=sim.tileIndex.at({90,146});selectedRide=sim.rides.empty()?-1:0;buildType=-1;if(selectedRide>=0)gameFocus(sim.rides[0].slot);accumulator=0;gameNotice="Saved park restored, including guests and queues.";}
static GameAsset makeGameAsset(int id){
    auto it=std::find_if(catalog.begin(),catalog.end(),[&](const Entry&e){return e.id==id;});require(it!=catalog.end(),"Playtest asset missing");
    GameAsset a;a.model=loadModel(*it);a.animation=loadAnimation(id,a.model.nodes.size());a.idle=a.animation;
    // Separate operating candidates from the four code-proven idle bindings.
    std::ifstream operating(root/L"extracted/runtime/operating_animations.tsv");std::string line;
    while(std::getline(operating,line)){if(line.empty()||line[0]=='#')continue;int modelID,group;std::string path;std::istringstream row(line);require(bool(row>>modelID>>group>>path),"Invalid operating catalog");if(modelID!=id)continue;auto original=animationCatalog.find(id);bool existed=original!=animationCatalog.end();std::pair<int,fs::path> previous;if(existed)previous=original->second;animationCatalog[id]={group,assetPath(path)};a.animation=loadAnimation(id,a.model.nodes.size());if(existed)animationCatalog[id]=previous;else animationCatalog.erase(id);break;}
    a.minZ=1e30f;
    float lx=1e30f,ly=1e30f,hx=-1e30f,hy=-1e30f;
    for(auto&p:a.model.parts){texture(p.texture);if(a.model.nodes[p.node].locator)continue;for(const auto&v:p.vertices){auto q=point(a.model.nodes[p.node].world,v);a.minZ=std::min(a.minZ,q[2]);lx=std::min(lx,q[0]);hx=std::max(hx,q[0]);ly=std::min(ly,q[1]);hy=std::max(hy,q[1]);}}a.spanX=hx-lx;a.spanY=hy-ly;a.centerX=(lx+hx)/2;a.centerY=(ly+hy)/2;return a;
}
static void initGame(){
    setPark(true);grid=false;wire=false;
    std::set<playtest::Tile> points;for(const auto&i:scene)if(i.kind=='P')points.insert({int(std::lround(i.x/32-.5f)),int(std::lround(-i.y/32-.5f))});sim.graph(points);
    // Append the entrance walkway so old saved path indices stay valid.
    for(int y=144;y<=148;y++){playtest::Tile t{90,y};int index=int(sim.tiles.size());sim.tiles.push_back(t);sim.tileIndex[t]=index;sim.edges.push_back({});for(auto d:std::array<playtest::Tile,4>{{{-1,0},{1,0},{0,-1},{0,1}}}){auto it=sim.tileIndex.find({t.x+d.x,t.y+d.y});if(it!=sim.tileIndex.end()){sim.edges[index].push_back(it->second);sim.edges[it->second].push_back(index);}}}
    sim.buses=true;

    std::ifstream layout(root/L"extracted/runtime/park_layout.tsv");std::string row;while(std::getline(layout,row)){if(row.empty()||row[0]!='S')continue;char tag;playtest::Slot s;std::istringstream in(row);require(bool(in>>tag>>s.id>>s.x>>s.y>>s.w>>s.h)&&s.w>0&&s.h>0,"Invalid build plot");sim.slots.push_back(s);}
    require(!sim.slots.empty(),"No build plots");
    std::ifstream ui(root/L"extracted/runtime/ui_assets.tsv");int uiID;std::string uiPath;while(ui>>uiID>>uiPath)uiAssets[uiID]=uiPath;
    std::ifstream bounds(root/L"extracted/runtime/ui_bounds.tsv");std::array<float,4> bb;while(bounds>>uiID>>bb[0]>>bb[1]>>bb[2]>>bb[3])uiBounds[uiID]=bb;
    std::ifstream types(root/L"extracted/runtime/attractions.tsv");int id,thumb;std::string category,name;
    while(types>>id>>category>>thumb){std::getline(types,name);if(!name.empty()&&name[0]=='\t')name.erase(0,1);
        auto a=makeGameAsset(id);float loX=1e30f,loY=1e30f,hiX=-1e30f,hiY=-1e30f;
        for(auto&p:a.model.parts)if(!a.model.nodes[p.node].locator)for(auto&v:p.vertices){auto pos=point(a.model.nodes[p.node].world,v);loX=std::min(loX,pos[0]);hiX=std::max(hiX,pos[0]);loY=std::min(loY,pos[1]);hiY=std::max(hiY,pos[1]);}
        auto footprint=[](float extent){float cells=extent/32;return cells<=1.05f?1:cells<=4.5f?4:cells<=6.5f?6:int(std::ceil(cells-.5f));};
        double duration=20;
        // The recovered map has square 4/6/16-tile ride plots. Geometry bounds can be rectangular.
        int size=footprint(std::max(hiX-loX,hiY-loY));if(size>6&&size<=16)size=16;
        sim.types.push_back({id,size,size,locatorCatalog.count(id)?int(locatorCatalog.at(id).size()):6,duration,name});
        sim.types.back().shop=category=="SHOPS";
        if(restoredRides.count(id)&&!sim.types.back().shop&&sim.types.back().capacity==0)sim.types.back().capacity=4;
        if(!thumb&&!sim.types.back().shop){
            thumb=10000+id;uiAssets[thumb]="runtime/shop_art/"+std::to_string(id)+".png";uiBounds[thumb]={0,0,1,1};
        }
        typeThumbs.push_back(thumb);typeCategories.push_back(category=="SHOPS");gameAssets.emplace(id,std::move(a));
    }
    require(sim.types.size()>100,"Original attraction catalog missing");
    sim.setLocalBalance();if(startupActive){startupProgress=.65f;startupMessage="Preparing mascots and park facilities";startupRender(true);}
    for(int id=718;id<=723;id++){
        animationCatalog[id]={id-521,assetPath("animations/ios/group_0"+std::to_string(id-521)+".tpa")};
        gameAssets.emplace(id,makeGameAsset(id));
    }
    for(int id=252;id<=277;id++)if(!gameAssets.count(id))gameAssets.emplace(id,makeGameAsset(id));
    for(int id=8;id<=207;id++)if(!gameAssets.count(id))gameAssets.emplace(id,makeGameAsset(id));
    for(int id=0;id<8;id++)gameAssets.emplace(id,makeGameAsset(id));
    for(int id=405;id<=411;id++)gameAssets.emplace(id,makeGameAsset(id));
    for(const auto&binding:fittedPassengerPose)if(!gameAssets.count(binding.second))gameAssets.emplace(binding.second,makeGameAsset(binding.second));
    std::ifstream passengerFile(root/L"extracted/runtime/passenger_poses.tsv");int passengerRide,variant,poseModel;
    while(passengerFile>>passengerRide>>variant>>poseModel){require(variant==int(passengerPoses[passengerRide].size()),"Invalid passenger pose order");passengerPoses[passengerRide].push_back(poseModel);if(!gameAssets.count(poseModel))gameAssets.emplace(poseModel,makeGameAsset(poseModel));}
    for(int id:{392,307,216,217,218,219,273,274,295,628,629,630,631,322,323,324,325,326,327,633,634,635,636,647,648,649,650,651,652,653,669,670,671})gameAssets.emplace(id,makeGameAsset(id));
    std::ifstream themeFile(root/L"extracted/runtime/territory_themes.tsv");int ti;TerritoryTheme theme;
    while(themeFile>>ti>>theme.statue>>theme.ground>>theme.path>>theme.fence4>>theme.fence6>>theme.fence16>>theme.gate>>theme.name){require(ti==int(territoryThemes.size()),"Invalid theme order");territoryThemes.push_back(theme);
        for(int id:{theme.statue,theme.ground,theme.fence4,theme.fence6,theme.fence16,theme.gate,theme.gate+1,theme.gate+2,theme.gate+3})if(id&&!gameAssets.count(id))gameAssets.emplace(id,makeGameAsset(id));
        for(int id=306;id<=321;id++){auto key=std::make_pair(id,theme.path);if(sceneMeshes.count(key))continue;auto entry=std::find_if(catalog.begin(),catalog.end(),[&](const Entry&e){return e.id==id;});SceneMesh mesh;mesh.model=loadModel(*entry);texture(theme.path);mesh.list=glGenLists(1);glNewList(mesh.list,GL_COMPILE);drawParts(mesh.model,theme.path);glEndList();sceneMeshes.emplace(key,std::move(mesh));}}
    require(territoryThemes.size()==12,"Missing theme catalog");
    std::ifstream plotFile(root/L"extracted/runtime/theme_plots.tsv");playtest::Slot tp;
    while(plotFile>>tp.id>>tp.x>>tp.y>>tp.w>>tp.h)themePlots[tp.id]=tp;
    require(themePlots.size()==20,"Missing theme plots");
    for(int id:{16,17,18,19,20,21,22,23,24,25,26,27,331,332,333,334,638,639,640,641,642,643,644,645,646})if(!gameAssets.count(id))gameAssets.emplace(id,makeGameAsset(id));
    std::ifstream binFile(root/L"extracted/runtime/trash_bins.tsv");playtest::TrashBin bin;
    while(binFile>>bin.x>>bin.y>>bin.territory){bin.remaining=sim.nextBinInterval();sim.bins.push_back(bin);}
    std::ifstream decs(root/L"extracted/runtime/decorations.tsv");int dm,dx,dy,territory;
    while(decs>>dm>>dx>>dy>>territory){if(!gameAssets.count(dm))gameAssets.emplace(dm,makeGameAsset(dm));parkDecorations.push_back({dm,(dx+.5f)*32,-(dy+.5f)*32});}
    // Local demonstration starting area, not recovered ownership or spawn logic.
    double best=1e30;for(size_t i=0;i<sim.tiles.size();i++){auto t=sim.tiles[i];double d=std::pow(t.x-90.,2)+std::pow(t.y-146.,2);if(d<best){best=d;sim.spawn=int(i);}}
    std::vector<int> plots;for(size_t i=0;i<sim.slots.size();i++)plots.push_back(int(i));std::sort(plots.begin(),plots.end(),[](int a,int b){auto&s=sim.slots[a];auto&t=sim.slots[b];return std::hypot(s.x-90.,s.y-131.)<std::hypot(t.x-90.,t.y-131.);});
    for(int type:{4,3,2,0}){int bestSlot=-1;double score=1e30;for(int slot:plots)if(sim.territory(slot)==17&&sim.canBuild(slot,type)){const auto&s=sim.slots[slot];double d=s.w*s.h*10000.+std::hypot(s.x-90.,s.y-131.);if(d<score){score=d;bestSlot=slot;}}if(bestSlot>=0)sim.build(bestSlot,type);}
    for(size_t i=0;i<sim.slots.size();i++)if(sim.occupant(int(i))<0&&sim.slots[i].id%3!=0)sim.lockedSlots.insert(int(i));
    sim.enforceTerritories=true;sim.openTerritories.insert(17);for(auto&r:sim.rides)sim.openTerritories.insert(sim.territory(r.slot));
    sim.adaptiveCrowd=true;require(!sim.rides.empty(),"No connected plot fits the ride models");for(int i=0;i<4;i++)sim.addGuest();
    selectedRide=0;gameFocus(sim.rides[0].slot);sim.validate();
    auto originalFont=assetPath("fonts/Theme_Park_1.8.3_ios_5.1.1/members/Payload/ThemePark.app/ITCKabelStd-Bold.otf");AddFontResourceExW(originalFont.c_str(),FR_PRIVATE,nullptr);
    HFONT font=CreateFontW(-20,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,FF_DONTCARE,L"ITC Kabel Std");
    HGDIOBJ old=SelectObject(dc,font);gameFont=glGenLists(256);require(gameFont&&wglUseFontBitmapsA(dc,0,256,gameFont),"Game font failed");SelectObject(dc,old);DeleteObject(font);
    startingPark=sim;
    if(!testing)try{if(fs::is_regular_file(gameSavePath()))gameLoad();else{auto previous=gameSavePath().parent_path()/L"playtest-v6.txt";if(!fs::is_regular_file(previous))previous=gameSavePath().parent_path()/L"playtest-v5.txt";if(!fs::is_regular_file(previous))previous=gameSavePath().parent_path()/L"playtest-v4.txt";if(!fs::is_regular_file(previous))previous=gameSavePath().parent_path()/L"playtest-v3.txt";if(!fs::is_regular_file(previous))previous=gameSavePath().parent_path()/L"playtest-v2.txt";if(fs::is_regular_file(previous)){std::ifstream in(previous);sim.read(in);if(previous.filename()!=L"playtest-v5.txt"&&previous.filename()!=L"playtest-v6.txt")for(size_t i=0;i<sim.slots.size();i++)if(sim.occupant(int(i))<0&&sim.slots[i].id%3!=0)sim.lockedSlots.insert(int(i));sim.spawn=sim.tileIndex.at({90,146});selectedRide=sim.rides.empty()?-1:0;if(selectedRide>=0)gameFocus(sim.rides[0].slot);gameNotice="Previous park imported. Bouncy Castle is now available to build.";}}}catch(const std::exception&e){gameNotice=std::string("Save not loaded: ")+e.what();}
    
    if(startupActive){startupProgress=.95f;startupMessage="Restoring your saved park";startupRender(true);}SetWindowTextW(window,L"Theme Park | Offline Playtest");
    if(!testing)music.open(assetPath("runtime/audio/track1.mp3"));
}
static void gamePoseClip(GameAsset&a,const Animation&clip,double ms){for(size_t i=0;i<a.model.nodes.size();i++){auto&n=a.model.nodes[i];Mat local=transform(n.frame);if(!clip.tracks.empty())local=multiply(local,transform(sample(clip.tracks[i],ms,clip.frameMS)));n.world=multiply(n.parent<0?identity():a.model.nodes[n.parent].world,local);}}
static void gamePose(GameAsset&a,double ms){gamePoseClip(a,a.animation,ms);}
// Provisional operating motions, independent of the four recovered idle clips.
static bool gameMechanicalPose(GameAsset&a,double time,bool active){
    if(a.model.id==487){
        auto& n=a.model.nodes[1];auto f=n.frame;
        if(active)f[2]*=float(1+.035*std::sin(time*4));
        n.world=multiply(a.model.nodes[0].world,transform(f));return true;
    }
    int node=-1,axis=2;float angle=0;switch(a.model.id){
    case 409:node=1;axis=1;angle=float(.6*std::sin(time*1.5));break;
    case 410:node=2;axis=1;angle=float(.45*std::sin(time*1.6));break;
    case 430:node=1;angle=float(time*.9);break;
    case 445:node=1;axis=3;angle=float(-40*(1-std::cos(time*1.2)));break;
    case 449:node=3;angle=float(time*1.4);break;
    case 463:node=3;angle=float(time*.65);break;
    case 465:node=2;angle=float(time*.7);break;
    case 471:node=3;angle=float(time*.55);break;
    case 467:node=3;axis=3;angle=float(45*(1-std::cos(time*.7)));break;
    case 476:node=2;axis=1;angle=float(time*.35);break;
    case 511:node=1;angle=float(time*.65);break;
    default:return false;}
    if(!active)angle=0;
    for(size_t i=0;i<a.model.nodes.size();i++){auto&n=a.model.nodes[i];Mat local=transform(n.frame);if(int(i)==node||(a.model.id==449&&(i==6||i==9||i==12))){std::array<float,10> delta{1,1,1,0,0,0,1,0,0,0};if(axis==3)delta[9]=angle;else{delta[3+axis]=std::sin(angle/2);delta[6]=std::cos(angle/2);}local=multiply(local,transform(delta));}if(a.model.id==476&&n.parent==2){std::array<float,10> counter{1,1,1,0,float(-std::sin(angle/2)),0,float(std::cos(angle/2)),0,0,0};local=multiply(local,transform(counter));}n.world=multiply(n.parent<0?identity():a.model.nodes[n.parent].world,local);}return true;
}
static void drawGuestModel(int model,int variant);
static void gameThemedFence(const Model&m,float halfHeight,bool cutEntrance){
    if(!cutEntrance){drawParts(m);return;}
    // Original theme table includes closed plots. Cut the shared front opening
    // at the same native-coordinate anchor as the ride gate (-16 on X).
    glPushAttrib(GL_ENABLE_BIT);GLdouble left[]={-1,0,0,-32},right[]={1,0,0,0};
    glClipPlane(GL_CLIP_PLANE1,left);glEnable(GL_CLIP_PLANE1);drawParts(m);
    glClipPlane(GL_CLIP_PLANE1,right);drawParts(m);
    GLdouble middleLeft[]={1,0,0,32},middleRight[]={-1,0,0,0},back[]={0,1,0,halfHeight-10};
    glClipPlane(GL_CLIP_PLANE1,middleLeft);glClipPlane(GL_CLIP_PLANE2,middleRight);glClipPlane(GL_CLIP_PLANE3,back);glEnable(GL_CLIP_PLANE2);glEnable(GL_CLIP_PLANE3);drawParts(m);glPopAttrib();
}
static void gameConstruction(const playtest::Slot&s,const playtest::Ride&r){
    int base=s.w<=4?638:s.w<=6?641:644;float nominal=s.w<=4?4:s.w<=6?6:16;
    glPushMatrix();glTranslatef((s.x+s.w/2.f)*32,-(s.y+s.h/2.f)*32,0);glScalef(s.w/nominal,s.h/nominal,1);
    // Original scaffold is a stack of 32-unit storeys, capped by one intact roof.
    // Server buildHeight is unavailable; use three storeys as in the reference.
    drawParts(gameAssets.at(base).model);
    for(int level=0;level<3;level++){glPushMatrix();glTranslatef(0,0,level*32.f);drawParts(gameAssets.at(base+(level==2?1:2)).model);glPopMatrix();}
    float half=nominal*16-16;
    for(int level=0;level<3;level++){
        double age=std::max(0.,sim.buildSeconds(r.type)-r.buildRemaining);bool walking=std::fmod(age+level*3,10.)<5;
        // Stop on the walkway to hammer, instead of orbiting all workers at ground height.
        double travel=std::floor((age+level*3)/10.)*5+std::min(5.,std::fmod(age+level*3,10.));
        float edge=float(std::fmod(travel*12/(8*half)+level/3.,1.)*4);int side=int(edge);
        float along=(edge-side)*2*half-half;float x=side==0?along:side==1?half:side==2?-along:-half,y=side==0?-half:side==1?along:side==2?half:-along;
        int model=walking?23+int(age*9+level)%5:16+int(age*10+level)%7;
        glPushMatrix();glTranslatef(x,y,level*32.f);glRotatef(float(walking?side*90:side*90+90),0,0,1);drawParts(gameAssets.at(model).model);glPopMatrix();
    }
    glPopMatrix();
}
static void gameDrawRide(int type,int slot,int angle){
    auto&a=gameAssets.at(sim.types[type].model);auto&s=sim.slots[slot];
    int constructing=sim.occupant(slot);if(constructing>=0&&(sim.rides[constructing].buildRemaining>0||readyMarkers.count(slot))){gameConstruction(s,sim.rides[constructing]);return;}
    if(sim.types[type].model==404){gamePose(a,0);int occupant=sim.occupant(slot);
        if(occupant>=0&&sim.playing(sim.rides[occupant])&&sim.visualPassengers(sim.rides[occupant])>0)for(size_t i=5;i<=12;i++){auto&n=a.model.nodes[i];auto f=n.frame;f[2]*=float(1+.035*std::sin(sim.rides[occupant].animationTime*8+i*.4));n.world=multiply(n.parent<0?identity():a.model.nodes[n.parent].world,transform(f));}}
    int rideIndex=sim.occupant(slot);double motion=rideIndex>=0?sim.rides[rideIndex].animationTime:0;
    if(!a.animation.tracks.empty()&&sim.types[type].model!=404){bool active=rideIndex>=0&&sim.playing(sim.rides[rideIndex])&&sim.visualPassengers(sim.rides[rideIndex])>0;
        if(active)gamePose(a,motion*1000);else if(!a.idle.tracks.empty())gamePoseClip(a,a.idle,sim.time*1000);else gamePose(a,0);}
    else gameMechanicalPose(a,motion,rideIndex>=0&&sim.playing(sim.rides[rideIndex])&&sim.visualPassengers(sim.rides[rideIndex])>0);
    float rideScale=1,backOffset=0;
    if(sim.types[type].model!=404){rideScale=std::min({1.f,(s.w*32)/(angle%180?a.spanY:a.spanX),(s.h*32)/(angle%180?a.spanX:a.spanY)});backOffset=0;}
    glPushMatrix();glTranslatef(float((s.x+s.w/2.)*32),float(-(s.y+s.h/2.)*32)+backOffset,0);glRotatef(float(angle),0,0,1);glScalef(rideScale,rideScale,rideScale);
    drawParts(a.model);
    // Original Ride::playRidingAnim attaches to vis_loc nodes. The standing
    // passenger pose is provisional until server riding-animation IDs are recovered.
    if(rideIndex>=0&&sim.types[type].model!=404){
        auto markers=locatorCatalog.find(sim.types[type].model);
        if(markers!=locatorCatalog.end()){
            std::vector<std::pair<int,int>> seats;for(auto&node:markers->second)seats.push_back({node.second,node.first});std::sort(seats.begin(),seats.end());
            const auto&r=sim.rides[rideIndex];for(int i=0;i<sim.visualPassengers(r);i++){
                int seat=i,guest=i;
                for(int id:r.riders)if(sim.guests[id].seat==seat){guest=id;break;}
                if(seat>=int(seats.size()))continue;
                glPushMatrix();glMultMatrixf(a.model.nodes[seats[seat].second].world.data());auto poses=passengerPoses.find(sim.types[type].model);int guestModel=poses==passengerPoses.end()?gamePassengerModel(a.model.id):poses->second[guest%poses->second.size()];drawGuestModel(guestModel,poses==passengerPoses.end()?guest%10:0);glPopMatrix();
            }
        }
    }
    if(rideIndex>=0&&restoredRides.count(a.model.id)&&locatorCatalog[a.model.id].empty()&&sim.playing(sim.rides[rideIndex])){
        // Walk-through attractions without seat markers have a guest group at the entrance.
        for(int i=0;i<std::min(4,sim.visualPassengers(sim.rides[rideIndex]));i++){
            glPushMatrix();glTranslatef((i-1.5f)*11,-s.h*16.f+17,0);
            drawGuestModel(int(motion*6+i)%8,i);glPopMatrix();
        }
    }
    glPopMatrix();
    if(s.w==s.h){const auto&theme=territoryThemes.at(sim.themes[sim.territory(slot)]);int fence=s.w==4?theme.fence4:s.w==6?theme.fence6:s.w==16?theme.fence16:-1;if(fence>=0){glPushMatrix();glTranslatef(float((s.x+s.w/2.)*32),float(-(s.y+s.h/2.)*32),0);gameThemedFence(gameAssets.at(fence).model,s.h*16.f,false);glPopMatrix();}}
    glPushMatrix();glTranslatef(float((s.x+s.w/2.)*32-16),float(-(s.y+s.h)*32+16),.1f);if(gateFilm.slot==slot&&gateFilm.committed)glScalef(1,1,.1f+.9f*smoothStep(float((gateFilm.time-.85)/.42)));drawParts(gameAssets.at(territoryThemes.at(sim.themes[sim.territory(slot)]).gate+(sim.occupant(slot)>=0?sim.rides[sim.occupant(slot)].gateLevel:0)).model);glPopMatrix();
}
static void gamePlot(int index,float r,float g,float b){
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_CURRENT_BIT|GL_LINE_BIT);
    auto&s=sim.slots[index];glDisable(GL_TEXTURE_2D);glColor4f(r,g,b,.10f);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);glBegin(GL_QUADS);glVertex3f(s.x*32.f,-s.y*32.f,.5f);glVertex3f((s.x+s.w)*32.f,-s.y*32.f,.5f);glVertex3f((s.x+s.w)*32.f,-(s.y+s.h)*32.f,.5f);glVertex3f(s.x*32.f,-(s.y+s.h)*32.f,.5f);glEnd();
    glColor4f(r,g,b,1);glLineWidth(2);glBegin(GL_LINE_LOOP);glVertex3f(s.x*32.f,-s.y*32.f,1);glVertex3f((s.x+s.w)*32.f,-s.y*32.f,1);glVertex3f((s.x+s.w)*32.f,-(s.y+s.h)*32.f,1);glVertex3f(s.x*32.f,-(s.y+s.h)*32.f,1);glEnd();glPopAttrib();
}
struct RiderVisual{float x,y,z,angle;int model;};
static RiderVisual castleRider(const playtest::Ride&r,int seat,double remaining=-1){
    const auto&a=gameAssets.at(404);const auto&s=sim.slots[r.slot];(void)remaining;
    const auto&anchor=a.model.nodes[1+seat%4].frame;
    double phase=sim.playing(r)?std::fmod(r.animationTime*1.3+seat*.21,1.):0;
    double localX=anchor[7],localY=anchor[8],localZ=7+11*std::sin(phase*3.14159265);
    double theta=r.rotation*3.14159265/180;auto convert=[&](double x,double y,double z){x-=a.centerX;y-=a.centerY;return std::array<double,3>{(s.x+s.w/2.)*32+x*std::cos(theta)-y*std::sin(theta),-(s.y+s.h/2.)*32+x*std::sin(theta)+y*std::cos(theta),z};};
    auto dest=convert(localX,localY,localZ);
    int pose=405+int(phase*4)%4;
    return {float(dest[0]),float(dest[1]),float(dest[2]),float(r.rotation),pose};
}
static void drawGuestModel(int model,int variant){glMatrixMode(GL_TEXTURE);glPushMatrix();glTranslatef(float(variant%8)*.125f,float(variant/8)*.25f,0);glMatrixMode(GL_MODELVIEW);drawParts(gameAssets.at(model).model);glMatrixMode(GL_TEXTURE);glPopMatrix();glMatrixMode(GL_MODELVIEW);}
static void drawParkAsset(int model,float x,float y,float z=0){const auto&m=gameAssets.at(model).model;if(!gameVisible(x,y,m.radius+100,z))return;glPushMatrix();glTranslatef(x,y,z);drawParts(m);glPopMatrix();}
static void gameSoftDisc(float x,float y,float rx,float ry,float z,float r,float g,float b,float alpha,bool additive=false){
    if(alpha<.01f||!gameVisible(x,y,std::max(rx,ry),z))return;
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);glDisable(GL_TEXTURE_2D);glDisable(GL_ALPHA_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,additive?GL_ONE:GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);glBegin(GL_TRIANGLE_FAN);glColor4f(r,g,b,alpha);glVertex3f(x,y,z);for(int i=0;i<=40;i++){float a=i*6.2831853f/40;glColor4f(r,g,b,0);glVertex3f(x+rx*std::cos(a),y+ry*std::sin(a),z);}glEnd();glPopAttrib();
}
static void gameWorld(){
    for(int x=-17;x<=199;x+=36){glPushMatrix();glTranslatef(x*32.f,-154.5f*32.f,-5);glScalef(1,.64f,1);drawParts(gameAssets.at(392).model);glPopMatrix();}
    for(const auto&d:parkDecorations)drawParkAsset(d.model,d.x,d.y,.1f);
    for(size_t i=0;i<sim.tiles.size();i++){int area=sim.tileTerritory(int(i));if(!sim.openTerritories.count(area))continue;for(int next:sim.edges[i])if(!sim.openTerritories.count(sim.tileTerritory(next))){auto a=sim.tiles[i],b=sim.tiles[next];glPushMatrix();glTranslatef((a.x+b.x+1)*16.f,-(a.y+b.y+1)*16.f,1);glRotatef(a.x==b.x?0:90,0,0,1);glScalef(.5f,.5f,.5f);drawParts(gameAssets.at(10).model);glPopMatrix();}}
    // Original first territory office footprint: themeSlots row 0, (87,129), 8x6.
    for(const auto&entry:themePlots){int t=entry.first;if(!sim.openTerritories.count(t))continue;const auto&p=entry.second;float cx=(p.x+p.w/2.f)*32,cy=-(p.y+p.h/2.f)*32;int statue=territoryThemes.at(sim.themes[t]).statue;
        gameSoftDisc(cx,cy,p.w*18.f,p.h*18.f,.18f,.03f,.06f,.1f,.28f*gameDaylight());
        if(statue){auto&a=gameAssets.at(statue);float scale=std::min({1.f,p.w*32/a.spanX,p.h*32/a.spanY});glPushMatrix();glTranslatef(cx,cy,.2f-a.minZ*scale);glScalef(scale,scale,scale);drawParts(a.model);glPopMatrix();}
        else if(t==17){drawParkAsset(274+(sim.officeWings&3),cx,cy,.2f);if(sim.officeTheme<0)drawParkAsset(273,cx,cy,.3f);else{int base=252+sim.officeTheme*7;bool upper=sim.officeWings&4;drawParkAsset(base+(upper?1:0),cx,cy,.3f);if(upper)drawParkAsset(base+2,cx,cy,.3f);if(sim.officeWings&1)drawParkAsset(base+5,cx,cy,.3f);if(sim.officeWings&2)drawParkAsset(base+6,cx,cy,.3f);}}
        else {glPushMatrix();glTranslatef(cx,cy,.1f);glScalef(p.w/5.f,p.h/5.f,1);drawParts(gameAssets.at(651).model);glPopMatrix();}
    }

    drawParkAsset(633,91.5f*32.f,-144*32.f,0);
    // Road mesh is eight tiles long, including both sidewalks.
    for(int x=-17;x<199;x+=8)drawParkAsset(634,(x+4)*32.f,-147.5f*32.f,0);
    for(int x=-13;x<199;x+=8)drawParkAsset(636,x*32.f,-145.75f*32,0);
    for(int row=0;row<3;row++)for(int n=-7;n<81;n++)drawParkAsset(629,(n*2.5f+(row%2)*1.2f)*32,-(152+row*3.f)*32,0);
    // One continuous perimeter outside the path; the canal ends clip to this line.
    auto wallSegment=[](float x1,float y1,float x2,float y2){float length=std::hypot(x2-x1,y2-y1);int pieces=int(std::ceil(length/32));for(int n=0;n<pieces;n++){float a=float(n)/pieces,b=float(n+1)/pieces;if(!gameVisible(x1+(x2-x1)*(a+b)*.5f,y1+(y2-y1)*(a+b)*.5f,64))continue;glPushMatrix();glTranslatef(x1+(x2-x1)*(a+b)*.5f,y1+(y2-y1)*(a+b)*.5f,0);glRotatef(float(std::atan2(y2-y1,x2-x1)*180/3.14159265),0,0,1);glScalef(length/pieces/32,1,1);drawParts(gameAssets.at(323).model);glPopMatrix();}};
    float left=32,right=181*32,front=-145.25f*32,back=-32,gateX=91.5f*32,gateY=-144*32;
    wallSegment(left,front,gateX-75.2f,front);wallSegment(gateX+75.2f,front,right,front);
    wallSegment(gateX-75.2f,front,gateX-75.2f,gateY);wallSegment(gateX+75.2f,front,gateX+75.2f,gateY);
    wallSegment(left,back,right,back);wallSegment(left,front,left,back);wallSegment(right,front,right,back);
    drawParkAsset(635,87.5f*32,-145.75f*32,0);
    double busPhase=std::fmod(sim.time,40.);float busX=float(busPhase<10?139-48*busPhase/10:busPhase<25?91:91-52*(busPhase-25)/15);
    glPushMatrix();glTranslatef(busX*32,-147.5f*32.f,0);drawParts(gameAssets.at(295).model);glPopMatrix();

    if(buildType>=0)for(size_t i=0;i<sim.slots.size();i++)if(sim.canBuild(int(i),buildType,buildRotation))gamePlot(int(i),.3f,1,.6f);
    if(selectedRide>=0)gamePlot(sim.rides[selectedRide].slot,1,.8f,.12f);
    for(size_t i=0;i<sim.slots.size();i++)if(sim.occupant(int(i))<0&&sim.staffAt(int(i))<0&&sim.territoryOpen(int(i))){const auto&s=sim.slots[i];
        if(!gameVisible((s.x+s.w/2.f)*32,-(s.y+s.h/2.f)*32,s.w*24.f))continue;int model=sim.lockedSlots.count(int(i))?(s.w<=4?647:s.w<=6?648:649):(s.w==4?650:s.w==5?651:s.w==6?652:s.w==16?653:-1);
        if(model>=0){glPushMatrix();glTranslatef((s.x+s.w/2.f)*32,-(s.y+s.h/2.f)*32,0);float base=sim.lockedSlots.count(int(i))?(s.w<=4?4:s.w<=6?6:16):s.w;glScalef(s.w/base,s.h/base,1);drawParts(gameAssets.at(model).model);glPopMatrix();}}
    for(const auto&r:sim.rides){const auto&s=sim.slots[r.slot];float x=(s.x+s.w/2.f)*32,y=-(s.y+s.h/2.f)*32;if(!gameVisible(x,y,s.w*32.f+200))continue;gameSoftDisc(x+8,y-7,s.w*17.f,s.h*17.f,.18f,.05f,.08f,.12f,.24f*gameDaylight());gameDrawRide(r.type,r.slot,r.rotation);if(r.buildRemaining<=0)gameSoftDisc(x,y,s.w*27.f,s.h*27.f,.65f,1,.65f,.24f,.28f*(1-gameDaylight()),true);}
    if(buildType>=0&&hoverSlot>=0&&sim.occupant(hoverSlot)<0){bool valid=sim.canBuild(hoverSlot,buildType,buildRotation);gamePlot(hoverSlot,valid?.3f:1.f,valid?1.f:.2f,.3f);if(valid)gameDrawRide(buildType,hoverSlot,buildRotation);}
    if(!parade.path.empty()){
        // The horn, juggling and unicycle assemblies each contain two performers.
        const int members[]={718,719,719,722,720,721,723};
        for(int i=0;i<7;i++){
            double at=parade.time*1.8-i*.85;if(at<0)continue;
            auto pos=paradePosition(at);auto&a=gameAssets.at(members[i]);gamePose(a,parade.time*1000+i*83);
            glPushMatrix();glTranslatef(pos[0],pos[1],.2f);glRotatef(pos[2],0,0,1);drawParts(a.model);glPopMatrix();
        }
    }
    for(const auto&r:sim.rides)if(sim.playing(r)&&sim.types[r.type].model==404)for(int seat=0;seat<sim.visualPassengers(r);seat++){auto v=castleRider(r,seat);if(!gameVisible(v.x,v.y,40,v.z))continue;glPushMatrix();glTranslatef(v.x,v.y,v.z);glRotatef(v.angle,0,0,1);int guest=seat;for(int id:r.riders)if(sim.guests[id].seat==seat){guest=id;break;}drawGuestModel(v.model,guest%10);glPopMatrix();}
    for(const auto&bin:sim.bins)if(sim.openTerritories.count(bin.territory)){bool water=sim.themes[bin.territory]==11;drawParkAsset((water?333:331)+(bin.remaining<=0?1:0),(bin.x+.5f)*32,-(bin.y+.5f)*32,.1f);}
    if(staffPlacement)for(size_t i=0;i<sim.slots.size();i++){const auto&s=sim.slots[i];if(s.w==4&&s.h==4&&sim.territoryOpen(int(i))&&!sim.lockedSlots.count(int(i))&&sim.occupant(int(i))<0&&sim.staffAt(int(i))<0)gamePlot(int(i),.2f,.9f,.9f);}
    for(const auto&worker:sim.staff){const auto&s=sim.slots[worker.slot];gameSoftDisc((s.x+2)*32,-(s.y+2)*32,74,74,.18f,.03f,.06f,.1f,.28f*gameDaylight());auto&a=gameAssets.at(28+worker.kind);float scale=std::min(104/a.spanX,104/a.spanY);const auto&theme=territoryThemes.at(sim.themes[sim.territory(worker.slot)]);drawParkAsset(theme.fence4,(s.x+2)*32,-(s.y+2)*32,.1f);drawParkAsset(theme.gate,(s.x+1.5f)*32,-(s.y+3.5f)*32,.2f);glPushMatrix();glTranslatef((s.x+2)*32,-(s.y+2)*32,-a.minZ*scale);glScalef(scale,scale,scale);drawParts(a.model);glPopMatrix();
        auto t=sim.tiles[worker.tile];float x=t.x+.5f,y=t.y+.5f,angle=0;bool moving=worker.step+1<worker.route.size();if(moving){auto next=sim.tiles[worker.route[worker.step+1]];x+=(next.x-t.x)*worker.progress;y+=(next.y-t.y)*worker.progress;angle=float(std::atan2(-(next.y-t.y),next.x-t.x)*180/3.14159265)+90;}
        gameSoftDisc(x*32,-y*32,8,6,.3f,0,0,0,.25f);glPushMatrix();glTranslatef(x*32,-y*32,0);glRotatef(angle,0,0,1);drawParts(gameAssets.at(mascotBases[worker.kind]+(moving?int(worker.walkTime*10)%12:worker.bin>=0?int(sim.time*8)%12:0)*14).model);glPopMatrix();}
    for(size_t i=0;i<sim.guests.size();i++){const auto&g=sim.guests[i];if(g.state==playtest::State::OnBus)continue;if(g.state==playtest::State::Riding)continue;
        auto pos=sim.position(g);if(!gameVisible(float(pos.first),float(pos.second),40))continue;float angle=0;
        bool walking=g.state==playtest::State::Walking||g.state==playtest::State::Leaving;
        if(walking&&g.step+1<g.route.size()){auto a=sim.tiles[g.tile],b=sim.tiles[g.route[g.step+1]];angle=float(std::atan2(-(b.y-a.y),b.x-a.x)*180/3.14159265)+90;}
        // Queue offsets stay inside the entrance tile; not original queue geometry.
        if(g.state==playtest::State::Queued){const auto&q=sim.rides[g.target].queue;auto it=std::find(q.begin(),q.end(),int(i));int k=int(it-q.begin());pos.first+=(k%4-1.5)*5;pos.second+=(k/4-2)*4;}
        int frame=walking?int(std::fmod(g.walkTime/.1,8.)):0;
        gameSoftDisc(float(pos.first)+2,float(pos.second)-2,8,6,.3f,0,0,0,.25f);glPushMatrix();glTranslatef(float(pos.first),float(pos.second),0);glRotatef(angle,0,0,1);drawGuestModel(frame,int(i)%10);glPopMatrix();
    }
}
static void gameClouds(){
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,texture("models/textures/000001a2.png"));
    auto cloud=[](float x,float y,float size,float left,float right,float bottom,float top){
        if(!gameVisible(x,y,size,78))return;
        // Project the nontransparent part of the billboard back onto the ground.
        // Inset its centre so the rounded silhouette stays inside its own bank.
        float a=yaw*.017453293f,p=pitch*.017453293f,c=std::cos(a),s=std::sin(a);
        float vertical=.25f*size/std::max(.15f,std::cos(p)),shift=78*std::tan(p);
        float rx=std::abs(c)*size*.5f+std::abs(s)*vertical,ry=std::abs(s)*size*.5f+std::abs(c)*vertical;
        float fit=std::min({1.f,(right-left)/(2*rx+1),(top-bottom)/(2*ry+1)});size*=fit;rx*=fit;ry*=fit;
        x=std::clamp(x+s*shift,left+rx,right-rx)-s*shift;
        y=std::clamp(y+c*shift,bottom+ry,top-ry)-c*shift;
        glPushMatrix();glTranslatef(x,y,78);glRotatef(-yaw,0,0,1);glRotatef(pitch,1,0,0);glColor4f(1,1,1,1);glBegin(GL_QUADS);glTexCoord2f(0,1);glVertex3f(-size/2,-size*.35f,0);glTexCoord2f(1,1);glVertex3f(size/2,-size*.35f,0);glTexCoord2f(1,0);glVertex3f(size/2,size*.35f,0);glTexCoord2f(0,0);glVertex3f(-size/2,size*.35f,0);glEnd();glPopMatrix();};
    for(int t=0;t<20;t++)if(!sim.openTerritories.count(t)){
        for(int row=0;row<3;row++)for(int col=0;col<3;col++){
            unsigned h=(unsigned(t*9+row*3+col)+1)*2654435761u;
            float dx=int((h>>8)%103)-51,dy=int((h>>17)%83)-41;
            cloud((1+t%5*36+6+col*12)*32+dx,-(1+t/5*36+6+row*12)*32+dy,570+h%90,(1+t%5*36)*32.f,(37+t%5*36)*32.f,-(37+t/5*36)*32.f,-(1+t/5*36)*32.f);
        }
    }
    for(int y=-8500;y<=4200;y+=440)for(int x=-4200;x<=10000;x+=440)if(x<0||x>5824||y>0||y< -5260)cloud(float(x+(y/440%2)*70),float(y),670,x>5824?5824.f:-1e6f,x<0?0.f:1e6f,y>0?0.f:-1e6f,y< -5260?-5260.f:1e6f);
    glPopAttrib();
}
static void gameRect(int x,int y,int w,int h,float r,float g,float b,float alpha=1){glDisable(GL_TEXTURE_2D);glColor4f(r,g,b,alpha);glBegin(GL_QUADS);glVertex2i(x,y);glVertex2i(x+w,y);glVertex2i(x+w,y+h);glVertex2i(x,y+h);glEnd();}
static void gameLabel(int x,int y,const std::string&s){text(x,y,s,.09f,.23f,.27f);}
static void gameButton(int x,int y,int w,int h,const std::string&label,int action,bool active=false){
    gameRect(x,y+3,w,h,.04f,.32f,.36f);gameRect(x,y,w,h,active?1.f:.02f,active?.72f:.55f,active?.16f:.62f);text(x+12,y+h/2+6,label,active?.12f:1.f,active?.23f:1.f,active?.25f:1.f);gameButtons.push_back({x,y,w,h,action});
}
static void gameDebugHUD(){
    GLuint old=fontBase;fontBase=gameFont;gameButtons.clear();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    gameRect(14,14,236,58,1,.77f,.13f);gameLabel(30,39,"THEME PARK");gameLabel(30,60,"OFFLINE PLAYTEST");
    gameRect(262,14,540,58,1,.95f,.78f,.96f);gameLabel(278,38,std::to_string(sim.guests.size())+" guests    "+std::to_string(sim.rides.size())+" rides    "+std::to_string(sim.served)+" visits completed");
    std::ostringstream cameraInfo;cameraInfo<<std::fixed<<std::setprecision(2)<<"Zoom "<<zoom<<" -> "<<gameZoomTarget<<" | Tilt "<<90-pitch<<" | Yaw "<<yaw<<" | Radius "<<sceneRadius;gameRect(262,82,780,32,1,.95f,.78f,.96f);gameLabel(274,105,cameraInfo.str());
    int walking=0,queued=0,riding=0;for(auto&g:sim.guests){walking+=g.state==playtest::State::Walking;queued+=g.state==playtest::State::Queued;riding+=g.state==playtest::State::Riding;}
    gameLabel(278,60,"Walking "+std::to_string(walking)+"   Queuing "+std::to_string(queued)+"   On rides "+std::to_string(riding));
    gameButton(width-342,23,92,38,sim.paused?"Resume":"Pause",10);gameButton(width-238,23,90,38,std::to_string(sim.speed)+"x speed",11);gameButton(width-136,23,120,38,"+ 8 guests",12);
    gameRect(14,88,222,478,1,.94f,.76f,.97f);gameLabel(28,116,"BUILD A RIDE");gameLabel(28,139,"Pick a ride and a plot");
    for(size_t i=0;i<std::min(size_t(5),sim.types.size());i++){auto&t=sim.types[i];int y=151+int(i)*66;gameButton(26,y,198,38,t.name,100+int(i),buildType==int(i));gameLabel(34,y+57,std::to_string(t.w)+" x "+std::to_string(t.h)+" tiles  |  "+std::to_string(t.capacity)+" seats");}
    gameButton(26,486,94,34,"Find plot",13);gameButton(130,486,94,34,"Rotate",14);gameLabel(28,546,buildType<0?"Sandbox: no build costs":"Click green plot to build");
    gameRect(14,578,222,91,1,.94f,.76f,.97f);gameButton(26,587,104,32,music.enabled?"Music: on":"Music: off",22);gameButton(140,587,84,32,"Next",23);
    gameButton(26,629,198,30,"Volume: "+std::to_string(music.volume)+"% / track "+std::to_string(music.track),24);
    if(selectedGuest>=0){const auto&g=sim.guests[selectedGuest];int x=width-262;const char*states[]={"Walking to a ride","Waiting in queue","Enjoying the ride","Returning to entrance","Choosing next ride","Travelling by bus","Waiting for departure"};
        gameRect(x,88,248,320,1,.94f,.76f,.97f);gameLabel(x+16,116,"VISITOR "+std::to_string(selectedGuest+1));gameLabel(x+16,150,states[int(g.state)]);
        gameLabel(x+16,186,"Rides this visit: "+std::to_string(g.visits));gameLabel(x+16,222,"Destination:");
        gameLabel(x+16,246,g.target>=0?sim.types[sim.rides[g.target].type].name:"Park entrance / paths");gameLabel(x+16,282,"Camera follows this guest");
        gameButton(x+16,306,216,36,"Next guest",20);gameButton(x+16,354,216,36,"Back to ride",21);
    }else if(selectedRide>=0){auto&r=sim.rides[selectedRide];auto&t=sim.types[r.type];int x=width-262;
        gameRect(x,88,248,320,1,.94f,.76f,.97f);gameLabel(x+16,116,t.name);gameLabel(x+16,146,!r.open?"CLOSED":r.timer>0?"RIDE IN PROGRESS":"OPEN / BOARDING");
        gameLabel(x+16,178,"Queue: "+std::to_string(r.queue.size())+" / 24");gameLabel(x+16,204,"On board: "+std::to_string(r.riders.size())+" / "+std::to_string(t.capacity));
        gameLabel(x+16,230,"Completed: "+std::to_string(r.served));gameRect(x+16,247,216,10,.76f,.79f,.66f);gameRect(x+16,247,int(216*(r.timer>0?1-r.timer/t.duration:0)),10,.02f,.59f,.63f);
        gameLabel(x+16,283,r.timer>0?std::to_string(int(std::ceil(r.timer)))+" seconds remaining":"Waiting for guests");
        gameButton(x+16,299,102,38,r.open?"Close":"Open",15);gameButton(x+130,299,102,38,"Focus",16);
        gameButton(x+16,354,216,36,"View a guest",20);
    }
    gameRect(14,height-112,width-28,34,1,.96f,.82f,.95f);gameLabel(26,height-89,gameNotice.substr(0,size_t(std::max(20,(width-65)/8))));
    gameRect(0,height-66,width,66,.015f,.35f,.39f,.97f);gameButton(16,height-54,100,38,"Save",17);gameButton(128,height-54,100,38,"Load",18);gameButton(240,height-54,120,38,"Cancel build",19);
    text(382,height-37,"Right-drag: pan  |  Wheel: zoom  |  Q/E: orbit  |  R: rotate plot",1,.96f,.77f);
    text(382,height-15,"Original assets. Playtest rules, timings and UI are provisional.",.71f,.9f,.88f);
    glDisable(GL_BLEND);fontBase=old;
}
#include "park_original_ui.h"
static int gamePickSlot(int sx,int sy){
    float extent=1.35f/zoom,aspect=float(width)/height;double x=(2.*sx/width-1)*extent*aspect-panX;
    double y=((1-2.*sy/height)*extent-panY)/std::cos(pitch*3.14159265/180);double a=yaw*3.14159265/180;
    double wx=(x*std::cos(a)+y*std::sin(a))*sceneRadius+sceneCenter[0];double wy=(-x*std::sin(a)+y*std::cos(a))*sceneRadius+sceneCenter[1];
    for(size_t i=0;i<sim.slots.size();i++){const auto&s=sim.slots[i];if(wx>=s.x*32&&wx<(s.x+s.w)*32&&-wy>=s.y*32&&-wy<(s.y+s.h)*32)return int(i);}return -1;
}
static int gamePickThemePlot(int sx,int sy){
    float extent=1.35f/zoom,aspect=float(width)/height;double x=(2.*sx/width-1)*extent*aspect-panX;
    double y=((1-2.*sy/height)*extent-panY)/std::cos(pitch*3.14159265/180);double a=yaw*3.14159265/180;
    double wx=(x*std::cos(a)+y*std::sin(a))*sceneRadius+sceneCenter[0];double wy=(-x*std::sin(a)+y*std::cos(a))*sceneRadius+sceneCenter[1];
    for(const auto&entry:themePlots){const auto&s=entry.second;if(wx>=s.x*32&&wx<(s.x+s.w)*32&&-wy>=s.y*32&&-wy<(s.y+s.h)*32)return entry.first;}return -1;
}
static bool gamePanel(int x,int y){if(confirmRestart||officePanel||shopOpen||territoryPanel>=0||themePanel>=0)return true;if(!debugHUD)return (x<260&&y<150)||(x>width-85&&y<85)||y>height-65||(x<220&&y>height-80)||(x<80&&y>height-225)||((rideDetails||buildType>=0)&&x>260&&y<80)||(rideDetails&&((x>220&&y>height-155)||(x>170&&x<620&&y>height-240)))||(settingsOpen&&x<310&&y>height-445);return y<80||y>height-120||(x<242&&y<675)||(selectedRide>=0&&x>width-270&&y<420);}
static void gameFindPlot(){
    if(buildType<0){gameNotice="Select a ride card first.";return;}
    int best=-1;double distance=1e30;for(size_t i=0;i<sim.slots.size();i++)if(sim.canBuild(int(i),buildType,buildRotation)){auto&s=sim.slots[i];double d=s.w*s.h*10000.+std::hypot((s.x+s.w/2.)*32-sceneCenter[0],-(s.y+s.h/2.)*32-sceneCenter[1]);if(d<distance){distance=d;best=int(i);}}
    if(best>=0){gameFocus(best);gameNotice="A connected plot is centered. Click its green outline to build.";}else gameNotice="No connected empty plot fits this ride.";
}
static void gameAction(int action){
    try{
        if(confirmRestart&&action!=97&&action!=98&&action!=19)return;
        if(action==96){confirmRestart=true;return;}
        if(action==98){confirmRestart=false;return;}
        if(action==97&&confirmRestart){
            parade={};gateFilm={};readyMarkers.clear();sim=startingPark;gameExitDetail();selectedRide=0;buildType=hoverSlot=shopSlot=themePanel=territoryPanel=-1;
            staffPlacement=officePanel=shopOpen=settingsOpen=confirmRestart=false;
            accumulator=0;rewardBursts.clear();gameFocus(sim.rides.front().slot);gameSave();gameButtons.clear();return;
        }
        if(action==95&&staffDetail>=0){if(sim.sellStaff(staffDetail)){gameExitDetail();gameSound("sell");gameSave();}return;}
        if(action>=40000){readyMarkers.erase(action-40000);return;}
        if(action>=20000){if(sim.cleanSpill(action-20000))gameSound("collect");return;}
        if(rideDetails&&((action>=100&&action<2000)||action==20||action==13||action==14||(action>=2000&&action<4000&&action!=2000+selectedRide)))return;
        if(action<2000&&action!=42&&action!=37&&action!=41&&action!=43&&action!=47)gameSound("click");
        if(action>=60&&action<72){themeSelection=action-60;return;}
        if(action>=90&&action<95){staffKind=action-90;return;}
        if(action==75){if(shopSlot>=0){if(sim.buyStaff(shopSlot,staffKind)){shopOpen=false;gameSound("build");gameSave();}else gameNotice="Staff House needs an unlocked empty 4 x 4 plot and $1800.";}else{staffPlacement=true;shopOpen=false;buildType=-1;gameNotice="Choose an unlocked 4 x 4 plot for Staff House ($1800).";}return;}
        if(action==76){officePanel=true;focusBuilding(themePlots.at(17),true);settingsOpen=false;shopOpen=false;return;}if(action==77){officePanel=false;gameExitDetail();return;}
        if(action>=80&&action<=82){sim.officeTheme=action-80;gameSave();return;}if(action>=83&&action<=85){if(sim.extendOffice(action-83)){gameSound("build");gameSave();}return;}
        if(action==74){dayNightEnabled=!dayNightEnabled;return;}
        if(action==72){if(themePanel>=0&&sim.setTheme(themePanel,themeSelection)){gameSound("build");themePanel=-1;gameExitDetail();gameSave();}return;}
        if(action==73){themePanel=-1;gameExitDetail();return;}
        if(action>=4000){if(sim.pickup(action-4000))gameSound("collect");return;}
        if(action>=3000){if(sim.cleanBin(action-3000,true))gameSound("collect");return;}
        if(action>=2000){if(sim.releaseProfit(action-2000))gameSound("collect");return;}
        if(action>=1000){shopSelection=action-1000;return;}
        if(action>=100&&action<1000){selectedGuest=-1;buildType=action-100;buildRotation=0;gameNotice="Choose a green plot for "+sim.types[buildType].name+". Use Find plot if needed.";return;}
        switch(action){case 30:shopPage=0;staffPlacement=false;officePanel=false;if(rideDetails)gameExitDetail();shopOpen=!shopOpen;shopSlot=-1;buildType=-1;break;
        case 31:shopCategory=0;shopPage=0;break;case 32:if(shopSlot<0||sim.slots[shopSlot].w!=16){shopCategory=1;shopPage=0;}break;case 78:shopCategory=2;shopPage=0;break;
        case 33:shopPage=std::max(0,shopPage-1);break;case 34:shopPage++;break;
        case 35:if(!shopFits(shopSelection)||(shopSlot>=0&&sim.lockedSlots.count(shopSlot)))break;shopOpen=false;buildType=shopSelection;buildRotation=0;if(shopSlot>=0){if(sim.types[buildType].w!=sim.slots[shopSlot].w)buildRotation=90;gameFocus(shopSlot);}else gameFindPlot();break;
        case 47:if(territoryPanel>=0&&sim.expand(territoryPanel)){territoryPanel=-1;gameSound("unlock");}break;case 48:territoryPanel=-1;break;
        case 38:if(shopSlot>=0&&sim.territoryOpen(shopSlot)){sim.lockedSlots.erase(shopSlot);gameNotice="Plot unlocked. Choose a matching attraction.";}break;
        case 36:debugHUD=!debugHUD;break;case 37:if(rideDetails)beginGateFilm(false);else if(selectedRide>=0)sim.upgrade(selectedRide);break;
        case 40:gameExitDetail();break;
        case 41:if(selectedRide>=0){sim.sell(selectedRide);gameSound("sell");selectedRide=-1;gameExitDetail();}break;
        case 42:if(selectedRide>=0&&sim.tap(selectedRide))gameSound("tap");break;
        case 49:sfx.enabled=!sfx.enabled;if(!sfx.enabled)sfx.stop();break;
        case 43:beginGateFilm(true);break;
        case 44:settingsOpen=!settingsOpen;break;
        case 45:gameNotice="Missions are not available yet.";break;
        case 46:gameNotice="Map overview is not available yet.";break;
        case 10:sim.paused=!sim.paused;break;case 11:sim.speed=sim.speed==1?2:sim.speed==2?4:1;break;
        case 12:for(int i=0;i<8;i++)sim.addGuest();gameNotice="Guests added at the local entrance (maximum 160).";break;
        case 13:gameFindPlot();break;case 14:buildRotation=(buildRotation+90)%360;gameNotice="Build orientation: "+std::to_string(buildRotation)+" degrees.";break;
        case 15:if(selectedRide>=0)sim.close(selectedRide);break;case 16:if(selectedRide>=0)gameFocus(sim.rides[selectedRide].slot);break;
        case 17:gameSave();break;case 18:gameLoad();break;case 19:confirmRestart=false;staffPlacement=false;officePanel=false;gameExitDetail();territoryPanel=themePanel=-1;settingsOpen=false;buildType=-1;gameNotice="Build cancelled. Click an occupied plot to inspect its ride.";break;
        case 20:if(!sim.guests.empty()){selectedGuest=(selectedGuest+1)%int(sim.guests.size());buildType=-1;zoom=gameZoomTarget=4.5f;panX=panY=0;gameNotice="Following a visitor. Right-drag stops following.";}break;
        case 21:selectedGuest=-1;if(selectedRide>=0)gameFocus(sim.rides[selectedRide].slot);break;
        case 22:music.enabled=!music.enabled;if(music.enabled&&!music.opened)music.open(assetPath("runtime/audio/track"+std::to_string(music.track)+".mp3"));else music.apply();break;
        case 23:music.track=music.track%3+1;music.open(assetPath("runtime/audio/track"+std::to_string(music.track)+".mp3"));break;
        case 24:music.volume=music.volume>=100?0:music.volume+20;music.apply();break;}
        if(!music.error.empty())gameNotice=music.error;
    }catch(const std::exception&e){gameNotice=std::string("Could not complete action: ")+e.what();}
}
static void gameClick(int x,int y){
    for(auto it=gameButtons.rbegin();it!=gameButtons.rend();++it){const auto&b=*it;if(x>=b.x&&x<b.x+b.w&&y>=b.y&&y<b.y+b.h){gameAction(b.action);return;}}if(gamePanel(x,y))return;if(rideDetails){if(selectedRide<0)return;int slot=gamePickSlot(x,y);if(slot>=0&&sim.occupant(slot)==selectedRide&&sim.tap(selectedRide))gameSound("tap");return;}
    int themeTerritory=gamePickThemePlot(x,y);if(themeTerritory==17){gameAction(76);return;}if(themeTerritory>=0){if(!sim.openTerritories.count(themeTerritory)){territoryPanel=themeTerritory;return;}themePanel=themeTerritory;focusBuilding(themePlots.at(themePanel),true);themeSelection=sim.themes[themePanel];shopOpen=false;buildType=-1;return;}
    int slot=gamePickSlot(x,y);if(slot<0){gameNotice="Choose an outlined build plot beside a path.";return;}if(!sim.territoryOpen(slot)){territoryPanel=sim.territory(slot);return;}if(staffPlacement){if(sim.buyStaff(slot,staffKind)){staffPlacement=false;gameSound("build");gameSave();}else gameNotice="Choose an empty, unlocked 4 x 4 plot; Staff House costs $1800.";return;}if(sim.staffAt(slot)>=0){staffDetail=slot;selectedRide=-1;buildType=-1;rideDetails=true;gameDetailFocus(slot);return;}int existing=sim.occupant(slot);
    if(existing>=0){if(rideDetails&&selectedRide==existing){if(sim.tap(existing))gameSound("tap");return;}gameSound("select");selectedGuest=-1;selectedRide=existing;if(sim.rides[existing].challenge==-2)sim.rides[existing].challenge=-1;buildType=-1;rideDetails=true;gameDetailFocus(slot);gameNotice="Tap to start a full 20-second session.";return;}
    if(buildType>=0){int index=sim.purchase(slot,buildType,buildRotation);if(index<0)gameNotice="Cannot build: plot too small, occupied, or disconnected from the entrance.";
        else{gameSound("build");selectedRide=index;buildType=-1;gameNotice="Ride opened. Guests have a 50% chance to enter when space is available.";}}
    else {shopSlot=slot;shopPage=0;shopOpen=true;for(size_t i=0;i<sim.types.size();i++)if(shopFits(int(i))){shopSelection=int(i);shopCategory=typeCategories[i];break;}}
}
static void gameClampCamera(){
    if(inspectionOpen()||cameraBusy()||!sim.enforceTerritories||sim.openTerritories.empty())return;
    float minZoom=std::max(4.f,2.7f*sceneRadius*(float(width)/height)/6200.f);gameZoomTarget=std::max(gameZoomTarget,minZoom);zoom=std::max(zoom,minZoom);
    double a=yaw*3.14159265/180,p=pitch*3.14159265/180;
    double x=sceneCenter[0]-sceneRadius*(panX*std::cos(a)+panY/std::cos(p)*std::sin(a));
    double y=sceneCenter[1]+sceneRadius*(panX*std::sin(a)-panY/std::cos(p)*std::cos(a));
    double best=1e30,bx=x,by=y;
    for(int t:sim.openTerritories){double left=(1+t%5*36)*32,right=(1+t%5*36+36)*32,top=-(1+t/5*36)*32,bottom=-(1+t/5*36+36)*32;
        if(t==17)bottom=-146*32;double qx=std::clamp(x,left,right),qy=std::clamp(y,bottom,top),d=std::hypot(qx-x,qy-y);if(d<best){best=d;bx=qx;by=qy;}}
    double dx=(sceneCenter[0]-bx)/sceneRadius,dy=(sceneCenter[1]-by)/sceneRadius;panX=float(dx*std::cos(a)-dy*std::sin(a));panY=float((dx*std::sin(a)+dy*std::cos(a))*std::cos(p));
}
static void gameTick(double elapsed){
    sfx.reap();int previousServed=sim.served;size_t previousRiders=0;for(auto&r:sim.rides)previousRiders+=r.riders.size();
    if(rideCamera.active){auto&c=rideCamera;c.elapsed+=std::min(elapsed,.25);float t=float(std::min(1.,c.elapsed/.85)),u=t*t*(3-2*t);
        for(int i=0;i<3;i++)sceneCenter[i]=c.from[i]+(c.to[i]-c.from[i])*u;
        yaw=c.yawFrom+(c.yawTo-c.yawFrom)*u;zoom=c.zoomFrom+(c.zoomTo-c.zoomFrom)*u;pitch=c.pitchFrom+(c.pitchTo-c.pitchFrom)*u;panX=c.panXFrom+(c.panXTo-c.panXFrom)*u;panY=c.panYFrom+(c.panYTo-c.panYFrom)*u;sceneRadius=c.radiusFrom+(c.radiusTo-c.radiusFrom)*u;gameZoomTarget=zoom;if(t>=1)c.active=false;
    }else if(!inspectionOpen()&&!cameraBusy()){float blend=float(1-std::exp(-10*std::min(elapsed,.25)));zoom+=(gameZoomTarget-zoom)*blend;pitch+=(gameTilt(zoom)-pitch)*blend;}
    for(auto&b:rewardBursts)b.age+=elapsed;rewardBursts.erase(std::remove_if(rewardBursts.begin(),rewardBursts.end(),[](const RewardBurst&b){return b.age>.65;}),rewardBursts.end());
    if(!rideCamera.active&&!rideDetails&&!cameraBusy())gameClampCamera();
    bool sessionRunning=selectedRide>=0&&selectedRide<int(sim.rides.size())&&sim.rides[selectedRide].challenge>=0;
    double beforeChallenge=sessionRunning?sim.rides[selectedRide].challenge:-1;
    std::vector<int> building;for(const auto&r:sim.rides)if(r.buildRemaining>0)building.push_back(r.slot);
    accumulator+=std::min(elapsed,.25)*sim.speed;while(accumulator>=.05){sim.tick(.05);accumulator-=.05;}
    for(int slot:building){int index=sim.occupant(slot);if(index>=0&&sim.rides[index].buildRemaining<=0)readyMarkers[slot]=sim.time+8;}
    for(auto it=readyMarkers.begin();it!=readyMarkers.end();)if(it->second<sim.time||sim.occupant(it->first)<0)it=readyMarkers.erase(it);else ++it;
    advanceGateFilm(std::min(elapsed,.25));advanceParade(sim.paused?0.:std::min(elapsed,.25));
    for(const auto&item:sim.pickedItems){
        double t=std::min(1.,item.age/.7);
        rewardBursts.push_back({float(item.x+item.dx*t),float(item.y+item.dy*t),item.cash,item.flags});
    }
    sim.pickedItems.clear();
    if(sessionRunning&&rideDetails&&beforeChallenge<16.5&&sim.rides[selectedRide].challenge>=16.5){++cheerCues;gameSound("cheer");}
    if(sessionRunning&&sim.rides[selectedRide].challenge==-2){sim.releaseProfit(selectedRide,false);gameSound("collect");}
    size_t currentRiders=0;for(auto&r:sim.rides)currentRiders+=r.riders.size();(void)previousServed;(void)previousRiders;(void)currentRiders;
}
