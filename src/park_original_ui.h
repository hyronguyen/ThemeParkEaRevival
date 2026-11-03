// Desktop interaction composed from the original iPad3 textures/atlas regions.
// Positions adapt to the window; original mobile menu code is not reproduced.
// Original attraction copy was absent from the recovered localization tables.
static std::string shopCaption(const playtest::Type&t){
    static const std::map<int,std::string> copy={
        {403,"Step through the gates of a stone castle."},{404,"Kick off your shoes and bounce between the castle towers."},
        {409,"Swing high above the deck aboard a pirate ship."},{410,"Take a spin around the fountain."},
        {416,"Follow the red dragon through the twists of its coaster track."},
        {425,"Pick a chute and race down into the splash pool."},{427,"Drift through the rocky bends beneath the skull."},
        {432,"Find a way through the winding hedges."},{449,"Climb into a teacup and spin around the saucers."},
        {463,"Float through the day while your worries drift away."},{465,"Take a turn on the carousel."},
        {476,"See the park from the top of the wheel."},{515,"Catch a wave and ride it back to shore."},
        {522,"Board a hollow log for a splashdown."},{531,"Circle the sandy island surrounded by water."},
        {414,"Follow the timber walkway into a rustic hideaway."},
        {415,"Explore a little woodland retreat tucked beneath a green roof."},
        {418,"Cross a field of snowy islands and icy blue channels."},
        {419,"Wander between rocky hills and a little trailside cabin."},
        {420,"Set out across the water beneath the tall timber frame."},
        {421,"Pick a side on a board of oversized coloured squares."},
        {422,"Follow the blue lagoon around its little islands."},
        {423,"Wind around a tower of ice-blue spirals."},
        {424,"Explore a grassy clearing dotted with little landmarks."},
        {428,"Find your way across a frosty white landscape."},
        {429,"Board beneath a bright canopy at the end of the garden path."},
        {430,"Take off from a circular launch pad in a little spacecraft."},
        {431,"Climb aboard the striped tower for a fairground ride."},
        {433,"Follow the winding path to the tower above the trees."},
        {434,"Head for the lookout at the end of the path."},
        {436,"Step into a dark arena lit with flashes of pink."},
        {438,"Visit a green fountain set in a formal garden."},
        {440,"Venture beneath the stare of a giant skull."},
        {441,"Enter a purple-lit skull attraction after dark."},
        {443,"Follow the roots of a towering tree into the clearing."},
        {444,"Look up at the giant figure standing over the entrance."},
        {445,"Approach the silver tower and its wide observation dish."},
        {447,"Head down to the bright blue pool beside the timber deck."},
        {450,"Take a seat around the giant desert cactus."},
        {452,"Step into an open arena lined with long timber rails."},
        {453,"Enter a sandy frontier courtyard through its wooden gateway."},
        {454,"Explore an island garden surrounded by blue water."},
        {455,"Follow the track into the mouth of a rocky cavern."},
        {456,"Spring into the centre of a brightly coloured arena."},
        {458,"Dodge other drivers on the bumper-car floor."},
        {460,"Trace the tight turns of a coiled circuit."},
        {461,"Discover the striped tents arranged around the showground."},
        {467,"Look up from the garden path to the tall central tower."},
        {469,"Circle the purple towers beside a bright blue lagoon."},
        {470,"Board a brightly painted boat tucked into its own pool."},
        {471,"Wander among small shelters along the garden paths."},
        {472,"Enter the colourful fairground hall through its red gate."},
        {473,"Explore a rugged patch of rocky terrain."},
        {474,"Step inside a long pavilion beneath a pale roof."},
        {475,"Follow the path beneath a glass pavilion."},
        {477,"See the park through the spokes of an ice-blue wheel."},
        {479,"Ride a festive wheel above a garden of snowy trees."},
        {484,"Stand beneath a giant crimson bloom at the end of the path."},
        {486,"Visit a soda-bottle centrepiece surrounded by ice cubes."},
        {487,"Follow the dark, twisting circuit around its centre."},
        {488,"Bounce inside a castle with soda bottles for towers."},
        {489,"Follow bright red rails across a snowy coaster course."},
        {490,"Set sail beneath an enormous pink shell."},
        {491,"Circle a glowing blue pool on a starry platform."},
        {492,"Explore a cluster of domes beneath a translucent canopy."},
        {493,"Follow the curve of a raised track over a bright red platform."},
        {495,"Rise through a colourful stack of circular decks."},
        {497,"Visit a shallow pool with an island at its centre."},
        {498,"Follow the arc of a rainbow above a field of clouds."},
        {499,"Circle a bright central island on a turquoise pond."},
        {500,"Make a lap around a winding red-and-white course."},
        {502,"Explore a sunken courtyard in a sandy landscape."},
        {503,"Follow a narrow circuit through a spiral of bends."},
        {508,"Walk the blue decks around a chain of pools."},
        {509,"Board beneath a silver arch on a red-striped platform."},
        {511,"Take a turn around the outstretched arms of a giant octopus."},
        {512,"Head to the base of a tall red tower topped with a star."},
        {513,"Discover a hidden pool surrounded by rocky cliffs."},
        {514,"Visit a giant red mushroom with a white-spotted cap."},
        {517,"Step inside a circular pavilion beneath a deep blue canopy."},
        {519,"Find your way through a garden of dark purple walls."},
        {520,"Enter a chequered arena with a bright red centre."},
        {521,"Follow a winding course through a series of sweeping turns."},
        {529,"Discover tiny islands scattered across a blue lagoon."},
        {533,"Trace a loop of dark track over a red-and-gold arena."},
        {534,"Cross a pool to reach the island under its pointed roof."},
        {535,"Wind through towering green fronds and curling roots."},
        {536,"Explore a colourful island inside a ring of water."},
        {537,"Climb around the curves of an icy spiral tower."},
        {538,"Look up through the stacked rings of a tall white tower."},
        {539,"Take to the water in a colourful enclosed pool."},
        {540,"Circle a colourful centrepiece between branching paths."},
        {541,"Explore a frozen landscape split by dark blue channels."},
        {542,"Step into a vivid pink arena beneath a blue-and-yellow canopy."},
        {543,"Circle a purple tower on a dark, jewel-coloured platform."},
        {544,"Follow the bends of a water course around its central island."},
        {576,"Visit the pumpkin workshop and its mechanical creations."}
    };
    auto it=copy.find(t.model);if(it!=copy.end())return it->second;
    if(t.shop)return "Give guests a place to stop at "+t.name+" between rides.";
    return t.capacity>0?"Welcomes up to "+std::to_string(t.capacity)+" guests for each ride.":"A themed attraction for your park.";
}
static void uiImage(int id,int x,int y,int w,int h,float u0=0,float v0=0,float u1=-1,float v1=1){
    auto it=uiAssets.find(id);if(it==uiAssets.end())return;
    if(id==0xf4&&u1<0){
        auto b=uiBounds.at(id);float us[]={b[0],b[0]+8/256.f,b[2]-8/256.f,b[2]},vs[]={b[1],b[1]+8/64.f,b[3]-8/64.f,b[3]};
        int edge=std::min({10,w/3,h/3}),xs[]={x,x+edge,x+w-edge,x+w},ys[]={y,y+edge,y+h-edge,y+h};
        for(int row=0;row<3;row++)for(int col=0;col<3;col++)uiImage(id,xs[col],ys[row],xs[col+1]-xs[col],ys[row+1]-ys[row],us[col],vs[row],us[col+1],vs[row+1]);return;
    }
    if(u1<0){auto b=uiBounds.at(id);u0=b[0];v0=b[1];u1=b[2];v1=b[3];}
    glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,texture(it->second));glColor4f(1,1,1,1);
    glBegin(GL_QUADS);glTexCoord2f(u0,v0);glVertex2i(x,y);glTexCoord2f(u1,v0);glVertex2i(x+w,y);glTexCoord2f(u1,v1);glVertex2i(x+w,y+h);glTexCoord2f(u0,v1);glVertex2i(x,y+h);glEnd();glDisable(GL_TEXTURE_2D);
}
static void uiAtlas(int sx,int sy,int sw,int sh,int x,int y,int w,int h){uiImage(0xb6,x,y,w,h,sx/2048.f,sy/1024.f,(sx+sw)/2048.f,(sy+sh)/1024.f);}
static void uiModel(int id,int x,int y,int w,int h,bool environment=false){
    const auto&m=gameAssets.at(id).model;glPushAttrib(GL_ALL_ATTRIB_BITS);glEnable(GL_SCISSOR_TEST);glScissor(x,height-y-h,w,h);glDepthMask(GL_TRUE);glClear(GL_DEPTH_BUFFER_BIT);glViewport(x,height-y-h,w,h);
    // Fit the projected silhouette, independent of the model origin and locator meshes.
    struct Frame{float left,right,bottom,top;};
    static std::map<int,Frame> frames;
    auto it=frames.find(id);
    if(it==frames.end()){
        Frame f{1e30f,-1e30f,1e30f,-1e30f};
        float a=-35*.017453293f,p=40*.017453293f;
        for(const auto&part:m.parts)if(!m.nodes[part.node].locator)for(const auto&v:part.vertices){
            auto q=point(m.nodes[part.node].world,v);
            float u=q[0]*std::cos(a)-q[1]*std::sin(a);
            float t=(q[0]*std::sin(a)+q[1]*std::cos(a))*std::cos(p)+q[2]*std::sin(p);
            f.left=std::min(f.left,u);f.right=std::max(f.right,u);f.bottom=std::min(f.bottom,t);f.top=std::max(f.top,t);
        }
        it=frames.emplace(id,f).first;
    }
    const auto&f=it->second;float aspect=float(w)/h,extent=.5f*std::max(f.top-f.bottom,(f.right-f.left)/aspect)*(environment?1.30f:1.025f);
    float cx=(f.left+f.right)*.5f,cy=(f.top+f.bottom)*.5f;
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();glOrtho(cx-extent*aspect,cx+extent*aspect,cy-extent,cy+extent,-10000,10000);
    glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();glRotatef(-40,1,0,0);glRotatef(-35,0,0,1);
    glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glEnable(GL_ALPHA_TEST);glAlphaFunc(GL_GREATER,.02f);
    if(environment){
        const auto&a=gameAssets.at(id);float side=std::max({128.f,a.spanX,a.spanY})*1.09f,half=side*.5f;
        glPushMatrix();glTranslatef(a.centerX,a.centerY,a.minZ-.8f);
        auto prop=[](int model,float x,float y,float z,float sx,float sy,float sz){
            glPushMatrix();glTranslatef(x,y,z);glScalef(sx,sy,sz);drawParts(gameAssets.at(model).model);glPopMatrix();
        };
        prop(392,0,0,-2,side/220,side/220,1);
        for(int k=-4;k<=4;k++){
            float r=side/7.f,t=k*r;
            for(int dir:{-1,1}){prop(307,t,dir*(half+r*.5f),-1,r/32,r/32,1);prop(307,dir*(half+r*.5f),t,-1,r/32,r/32,1);}
        }
        prop(669,0,0,0,side/128,side/128,side/128);
        float tree=side/290;for(int k=-2;k<=2;k++){prop(629,k*side*.42f,half+side*.37f,0,tree,tree,tree);prop(629,-half-side*.37f,k*side*.42f,0,tree,tree,tree);}
        glPopMatrix();
    }
    drawParts(m);glPopMatrix();glMatrixMode(GL_PROJECTION);glPopMatrix();glMatrixMode(GL_MODELVIEW);glPopAttrib();
}
struct UIText{GLuint texture;int w,h;};
static std::map<std::string,UIText> uiTextCache;
static Gdiplus::PrivateFontCollection*uiFonts=nullptr;
static void clearUIText(){for(auto&kv:uiTextCache)glDeleteTextures(1,&kv.second.texture);uiTextCache.clear();delete uiFonts;uiFonts=nullptr;}
static void uiText(int x,int y,const std::string&s,float r=1,float g=1,float b=1,float fontSize=23,int boxWidth=0){
    if(!uiFonts){uiFonts=new Gdiplus::PrivateFontCollection();uiFonts->AddFontFile(assetPath("fonts/Theme_Park_1.8.3_ios_5.1.1/members/Payload/ThemePark.app/ITCKabelStd-Bold.otf").c_str());}
    std::string key=s+std::to_string(r)+std::to_string(g)+std::to_string(b)+std::to_string(fontSize);
    auto it=uiTextCache.find(key);
    if(it==uiTextCache.end()){
        // Bound cached dynamic counters; font textures remain a session resource.
        if(uiTextCache.size()>512){for(auto&kv:uiTextCache)glDeleteTextures(1,&kv.second.texture);uiTextCache.clear();}
        Gdiplus::FontFamily family;int found=0;uiFonts->GetFamilies(1,&family,&found);
        if(!found){text(x,y,s,r,g,b);return;}
        std::wstring ws(s.begin(),s.end());Gdiplus::GraphicsPath path;
        int style=family.IsStyleAvailable(Gdiplus::FontStyleBold)?Gdiplus::FontStyleBold:Gdiplus::FontStyleRegular;
        path.AddString(ws.c_str(),int(ws.size()),&family,style,fontSize,Gdiplus::PointF(0,0),nullptr);
        Gdiplus::RectF bounds;path.GetBounds(&bounds);int tw=std::max(1,int(std::ceil(bounds.Width))+6),th=std::max(1,int(std::ceil(bounds.Height))+6);
        Gdiplus::Matrix shift;shift.Translate(3-bounds.X,3-bounds.Y);path.Transform(&shift);
        Gdiplus::Bitmap bmp(tw,th,PixelFormat32bppARGB);Gdiplus::Graphics graphics(&bmp);graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics.Clear(Gdiplus::Color(0,0,0,0));Gdiplus::Pen outline(Gdiplus::Color(255,255,255,245),2.5f);outline.SetLineJoin(Gdiplus::LineJoinRound);
        Gdiplus::SolidBrush fill(Gdiplus::Color(255,BYTE(r*255),BYTE(g*255),BYTE(b*255)));graphics.DrawPath(&outline,&path);graphics.FillPath(&fill,&path);
        Gdiplus::BitmapData data{};Gdiplus::Rect rect(0,0,tw,th);bmp.LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&data);
        std::vector<unsigned char> pixels(size_t(tw)*th*4);for(int yy=0;yy<th;yy++){auto row=(unsigned char*)data.Scan0+yy*data.Stride;for(int xx=0;xx<tw;xx++){size_t j=(size_t(yy)*tw+xx)*4;pixels[j]=row[xx*4+2];pixels[j+1]=row[xx*4+1];pixels[j+2]=row[xx*4];pixels[j+3]=row[xx*4+3];}}bmp.UnlockBits(&data);
        GLuint tex;glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tw,th,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
        it=uiTextCache.emplace(key,UIText{tex,tw,th}).first;
    }
    const auto&t=it->second;if(boxWidth>0)x+=(boxWidth-t.w)/2;glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,t.texture);glColor4f(1,1,1,1);int top=y-t.h+3;
    glBegin(GL_QUADS);glTexCoord2f(0,0);glVertex2i(x,top);glTexCoord2f(1,0);glVertex2i(x+t.w,top);glTexCoord2f(1,1);glVertex2i(x+t.w,top+t.h);glTexCoord2f(0,1);glVertex2i(x,top+t.h);glEnd();glDisable(GL_TEXTURE_2D);
}
static void uiHit(int x,int y,int w,int h,int action){gameButtons.push_back({x,y,w,h,action});}
static void uiRound(int x,int y,int action,int sx,int sy,int sw,int sh){uiImage(0xf2,x,y,62,62);uiAtlas(sx,sy,sw,sh,x+9,y+9,44,44);uiHit(x,y,62,62,action);}

static void uiCash(int x,int y,int w=44,int h=30){uiAtlas(890,378,218,145,x,y,w,h);}
static void uiFlag(int x,int y,int w=26,int h=28){uiAtlas(1506,589,60,60,x,y,w,h);}
static void uiTicket(int x,int y,int w=42,int h=28){uiAtlas(1694,565,150,117,x,y,w,h);}
static std::pair<int,int> uiWorld(float x,float y,float z){
    float a=yaw*3.14159265f/180,p=pitch*3.14159265f/180,ex=1.35f/zoom;
    x-=sceneCenter[0];y-=sceneCenter[1];z-=sceneCenter[2];float rx=x*std::cos(a)-y*std::sin(a),ry=x*std::sin(a)+y*std::cos(a);
    return {int(width*.5f+(rx/sceneRadius+panX)*height/(2*ex)),int(height*.5f-(ry*std::cos(p)/sceneRadius+z*std::sin(p)/sceneRadius+panY)*height/(2*ex))};
}
// Crisp scalable panels; original font, icons and illustrated buttons stay intact.
static void uiPanel(int x,int y,int w,int h,bool teal=false){
    auto rounded=[&](int xx,int yy,int ww,int hh,int radius,bool border){
        glBegin(GL_TRIANGLE_FAN);glColor4f(border?1.f:teal?.05f:1.f,border?1.f:teal?.68f:.72f,border?1.f:teal?.75f:.02f,1);glVertex2f(xx+ww*.5f,yy+hh*.5f);
        for(int corner=0;corner<=4;corner++){for(int step=0;step<=(corner==4?0:12);step++){
            int c=corner%4;float a=float((-90+c*90+step*7.5)*3.14159265/180),px=float(xx+(c==0||c==1?ww-radius:radius))+radius*std::cos(a),py=float(yy+(c<2? (c==0?radius:hh-radius):(c==2?hh-radius:radius)))+radius*std::sin(a);
            float t=std::clamp((py-yy)/hh,0.f,1.f);if(border)glColor4f(1,1,.9f,1);else glColor4f(teal?.03f:1.f,teal?.76f-.23f*t:.86f-.30f*t,teal?.83f-.23f*t:.02f,1);glVertex2f(px,py);
        }}glEnd();
    };
    rounded(x,y,w,h,14,true);rounded(x+2,y+2,w-4,h-4,12,false);
}
static void uiDisc(int x,int y,float rx,float ry,float r,float g,float b){
    glDisable(GL_TEXTURE_2D);glColor4f(r,g,b,1);glBegin(GL_TRIANGLE_FAN);glVertex2i(x,y);
    for(int i=0;i<=36;i++){float a=i*6.2831853f/36;glVertex2f(x+rx*std::cos(a),y+ry*std::sin(a));}glEnd();
}
static void uiPickupBadge(int x,int y,bool cash,float scale=1){
    int w=int((cash?76:53)*scale),h=int(50*scale);
    if(cash)uiCash(x-w/2,y-h/2,w,h);else uiFlag(x-w/2,y-h/2,w,h);
}
static void uiCleanIcon(int x,int y){
    uiAtlas(1477,139,210,246,x-30,y-38,60,70);
    uiAtlas(1469,400,137,193,x-19,y-33,38,47);
}
static void gameHUD(){
    if(cameraBusy()){
        gameButtons.clear();
        if(parade.follow){
            glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            uiPanel(width/2-170,18,340,48,true);
            uiText(width/2-170,51,"RANK "+std::to_string(sim.celebratedRank)+"  -  PARK PARADE",1,.78f,.12f,23,340);
        }
        if(gateFilm.slot>=0&&gateFilm.committed&&gateFilm.time<1.8){
            auto p=uiWorld(gateFilm.gate.center[0],gateFilm.gate.center[1],23);float phase=float((gateFilm.time-.85)/.95);
            for(int i=0;i<9;i++){float a=i*6.28318f/9;int x=int(p.first+(52+phase*72)*std::cos(a)),y=int(p.second+(36+phase*55)*std::sin(a)-phase*35);int size=int(22*std::sin(std::clamp(phase,0.f,1.f)*3.14159f));if(size>0)uiAtlas(1281,563,137,136,x-size/2,y-size/2,size,size);}
        }
        return;
    }
    if(debugHUD){gameDebugHUD();return;}
    GLuint old=fontBase;fontBase=gameFont;gameButtons.clear();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    if(!shopOpen&&staffDetail<0){
        for(size_t i=0;i<sim.rides.size();i++)if(sim.rides[i].profit>0&&(!rideDetails||int(i)==selectedRide)){const auto&s=sim.slots[sim.rides[i].slot];auto p=uiWorld((s.x+s.w/2.f)*32,-(s.y+s.h/2.f)*32,75);
            if(p.first<0||p.first>width||p.second<130||p.second>height-180)continue;
            uiAtlas(1477,139,210,246,p.first-34,p.second-52,68,80);uiCash(p.first-29,p.second-30,58,38);uiHit(p.first-38,p.second-55,76,85,2000+int(i));}
        for(size_t i=0;i<sim.loot.size();i++){const auto&item=sim.loot[i];if(rideDetails&&selectedRide>=0){const auto&s=sim.slots[sim.rides[selectedRide].slot];if(item.x<s.x*32||item.x>(s.x+s.w)*32||-item.y<s.y*32||-item.y>(s.y+s.h)*32)continue;}double t=std::min(1.,item.age/.7);auto p=uiWorld(float(item.x+item.dx*t),float(item.y+item.dy*t),float(45+100*t*(1-t)));if(p.first<65||p.first>width-65||p.second<150||p.second>height-90)continue;uiPickupBadge(p.first,p.second,item.cash!=0);uiHit(p.first-39,p.second-27,78,54,4000+int(i));}
    }
    if(!rideDetails&&!shopOpen&&themePanel<0&&territoryPanel<0){
        for(size_t i=0;i<sim.bins.size();i++){const auto&b=sim.bins[i];if(b.remaining>0||!sim.openTerritories.count(b.territory))continue;auto p=uiWorld((b.x+.5f)*32,-(b.y+.5f)*32,35);if(p.first<75||p.first>width-60||p.second<160||p.second>height-100)continue;uiCleanIcon(p.first,p.second-4);uiHit(p.first-26,p.second-30,52,52,3000+int(i));}
        for(int tile:sim.spills){
            auto t=sim.tiles[tile];auto p=uiWorld((t.x+.5f)*32,-(t.y+.5f)*32,1);
            uiDisc(p.first,p.second,14,6,.54f,.66f,.18f);uiDisc(p.first+8,p.second+3,6,3,.65f,.73f,.21f);
            uiHit(p.first-20,p.second-14,42,30,20000+tile);
        }
        for(const auto&entry:readyMarkers){const auto&s=sim.slots[entry.first];auto p=uiWorld((s.x+s.w/2.f)*32,-(s.y+s.h/2.f)*32,100);
            uiAtlas(1477,139,210,246,p.first-36,p.second-52,72,84);uiAtlas(0,362,193,193,p.first-29,p.second-46,60,56);
            uiHit(p.first-36,p.second-52,72,84,40000+entry.first);}
        for(const auto&r:sim.rides)if(r.buildRemaining>0){const auto&s=sim.slots[r.slot];auto p=uiWorld((s.x+s.w/2.f)*32,-(s.y+s.h/2.f)*32,60);uiPanel(p.first-55,p.second-36,110,40,true);uiText(p.first-55,p.second-10,"BUILD "+std::to_string(int(std::ceil(r.buildRemaining)))+"s",1,1,1,18,110);}
    }
    if(!rideDetails){
    uiPanel(8,8,244,83);uiAtlas(760,536,140,145,10,12,64,64);uiText(12,62,std::to_string(sim.rank()),1,.35f,0,sim.rank()<10?40:sim.rank()<100?33:25,60);uiText(78,34,"PARK RANK",.02f,.6f,.65f);
    gameRect(82,42,154,10,1,1,.88f);gameRect(84,44,150,6,.12f,.34f,.35f);gameRect(84,44,sim.maxRank()?150:int(150.f*sim.rankProgress()/sim.rankTarget()),6,.06f,.82f,.73f);uiText(78,78,sim.maxRank()?"MAX":std::to_string(sim.rankProgress())+"/"+std::to_string(sim.rankTarget()),.02f,.5f,.6f,20,130);uiFlag(216,60,24,22);
    uiPanel(8,96,244,49);uiText(14,129,sim.streak>=10?"A+":sim.streak>=6?"A":sim.streak>=2?"B":sim.streak?"C+":"C",.02f,.56f,.60f,23,43);uiText(58,128,"STREAK",.12f,.30f,.32f,16);uiText(122,129,"+"+std::to_string(sim.bonus())+"%",1,.35f,0);uiCash(185,105,49,30);gameRect(24,137,int(200*sim.bonus()/50.f),3,.05f,.8f,.65f);
    uiRound(width-76,10,11,910,535,125,160);
    uiRound(12,height-218,30,850,140,195,195);
    uiImage(0xf2,12,height-146,62,62);uiImage(0x5ec,23,height-138,40,46);uiHit(12,height-146,62,62,46);
    uiRound(12,height-76,44,1110,380,170,180);
    uiRound(80,height-76,45,1920,210,125,150);
    uiRound(148,height-76,20,1870,550,135,150);
    int walletW=std::min(width-234,404);uiPanel(222,height-68,walletW,58);uiCash(235,height-52,55,37);uiText(298,height-25,"$"+std::to_string(sim.money),.15f,.2f,.25f);
    uiTicket(222+walletW-130,height-56,47,33);uiText(222+walletW-75,height-29,std::to_string(sim.tickets),.15f,.2f,.25f);
    if(buildType>=0){uiPanel(270,12,std::max(220,width-370),56);uiText(288,48,sim.types[buildType].name+"  $"+std::to_string(sim.buildPrice(buildType)),.02f,.55f,.6f);}
    }
    if(rideDetails&&selectedRide>=0&&!shopOpen){auto&r=sim.rides[selectedRide];
        int headW=std::min(510,width-32),headX=(width-headW)/2;uiPanel(headX,12,headW,52,true);uiText(headX+12,47,sim.types[r.type].name,.09f,.29f,.32f,24,headW-66);uiAtlas(345,568,88,94,headX+headW-45,22,30,30);uiHit(headX+headW-52,14,48,48,40);
        int by=height-108;int infoX=std::max(10,(width-840)/2);uiPanel(infoX,by,300,78);uiCash(infoX+12,by+22);uiText(infoX+62,by+48,"20s = $"+std::to_string(sim.baseCash(r.type)+10*r.gateLevel),.1f,.2f,.25f);uiFlag(infoX+216,by+25);uiText(infoX+248,by+48,std::to_string(sim.baseFlags(r.type)+r.gateLevel),.1f,.2f,.25f);
        uiImage(0xf2,width-345,by-7,96,96);uiText(width-345,by+22,r.gateLevel>=3?"MAX":"UPGRADE",1,.35f,0,19,96);uiText(width-345,by+54,"$"+std::to_string(sim.upgradePrice(selectedRide)),.1f,.2f,.25f,21,96);uiHit(width-350,by-7,105,96,37);
        uiImage(0xf2,width-230,by-7,96,96);uiTicket(width-207,by+8,42,28);uiText(width-230,by+62,"1",.1f,.2f,.25f,23,96);uiHit(width-235,by-7,105,96,43);
        uiImage(0xf2,width-115,by-7,96,96);uiText(width-115,by+22,"SELL",1,.35f,0,21,96);uiText(width-115,by+57,"$"+std::to_string(sim.buildPrice(r.type)/2+50*r.gateLevel),.1f,.2f,.25f,19,96);uiHit(width-118,by-7,105,96,41);
        if(sim.types[r.type].capacity>0&&r.buildRemaining<=0){
        int mx=infoX+66,my=by-74,mw=275;uiPanel(mx,my,mw,42);float value=float(r.happiness/100);bool happy=r.happiness>=30&&r.happiness<=80;
        gameRect(mx+14,my+14,mw-28,14,.12f,.3f,.34f);gameRect(mx+14,my+14,int((mw-28)*value),14,happy?.1f:1.f,happy?.8f:.1f,.1f);
        uiImage(happy?0x10e:r.happiness<30?0x10c:0x110,mx+int((mw-42)*value),my-2,42,44);
        uiImage(0xf2,mx-62,my-8,62,62);uiImage(0x108,mx-54,my,46,46);uiHit(mx-62,my-8,62,62,42);uiHit(mx,my,mw,48,42);
        if(r.challenge<0)uiText(mx,my-12,r.challenge==-2?"FINISHED":"TAP TO PLAY",.02f,.55f,.6f,21,mw);
        }

    }
    if(staffDetail>=0){
        int index=sim.staffAt(staffDetail);
        if(index>=0){int kind=sim.staff[index].kind,w=std::min(530,width-30),x=(width-w)/2,y=height-144;
            uiPanel(x,y,w,124);uiImage(staffArtwork[kind],x+10,y+10,105,104);
            uiText(x+124,y+34,staffNames[kind],.02f,.48f,.52f,23);
            uiText(x+124,y+61,"Patrol, clean and collect",.12f,.26f,.29f,17);
            uiPanel(x+124,y+72,148,40,true);uiText(x+124,y+100,"SELL  $900",.1f,.27f,.29f,20,148);uiHit(x+124,y+72,148,40,95);
            uiPanel(x+w-116,y+72,104,40);uiText(x+w-116,y+100,"CLOSE",.5f,.2f,.06f,19,104);uiHit(x+w-116,y+72,104,40,40);
        }
    }
    if(settingsOpen&&!rideDetails){
        int x=18,y=height-416;uiPanel(x,y,296,326);
        uiText(x+20,y+40,"PARK SETTINGS",.02f,.51f,.57f,24);
        auto setting=[&](int row,int col,const std::string& label,int action,bool accent){
            int xx=x+12+col*136,yy=y+58+row*64;
            uiPanel(xx,yy,132,50,accent);uiText(xx,yy+33,label,.10f,.29f,.32f,18,132);uiHit(xx,yy,132,50,action);
        };
        setting(0,0,sfx.enabled?"SFX ON":"SFX OFF",49,sfx.enabled);
        setting(0,1,music.enabled?"MUSIC ON":"MUSIC OFF",22,music.enabled);
        setting(1,0,dayNightEnabled?"DAY / NIGHT":"DAY ONLY",74,dayNightEnabled);
        setting(1,1,"DEBUG  F1",36,false);setting(2,0,"SAVE PARK",17,true);
        setting(2,1,"CLOSE",44,false);uiPanel(x+12,y+252,268,54);
        uiText(x+12,y+287,"START A NEW PARK",.68f,.19f,.05f,20,268);uiHit(x+12,y+252,268,54,96);
    }
    if(shopOpen){
        if(shopSlot>=0&&sim.slots[shopSlot].w==16)shopCategory=0;
        gameRect(0,0,width,height,0,0,0,.35f);
        int w=std::min(width-32,1100),h=std::min(height-100,460),x=(width-w)/2,y=(height-h)/2;
        uiPanel(x,y,w,h);uiAtlas(345,568,88,94,x+w-48,y+12,32,32);uiHit(x+w-54,y+6,44,44,30);
        std::vector<int> items;for(size_t i=0;i<sim.types.size();i++)if(typeCategories[i]==shopCategory&&shopFits(int(i)))items.push_back(int(i));
        std::stable_sort(items.begin(),items.end(),[](int a,int b){return sim.buildPrice(a)<sim.buildPrice(b);});
        int count=std::max(1,(w-238)/150),cardW=(w-238)/count;
        int pages=std::max(1,(int(items.size())+count-1)/count);shopPage%=pages;
        if(!items.empty()){
            auto first=items.begin()+shopPage*count,last=items.begin()+std::min(int(items.size()),(shopPage+1)*count);
            if(std::find(first,last,shopSelection)==last)shopSelection=*first;
        }
        bool locked=shopSlot>=0&&sim.lockedSlots.count(shopSlot);
        int cy=y+h-132,pw=std::min(368,w/3),ph=cy-y-38,tx=x+pw+40,buttonX=x+w-156;
        if(!items.empty()){
            const auto&t=sim.types[shopSelection];uiPanel(x+18,y+18,pw+4,ph+4,true);if(typeThumbs[shopSelection])uiImage(typeThumbs[shopSelection],x+22,y+22,pw-4,ph-4);else uiModel(t.model,x+22,y+22,pw-4,ph-4,true);
            float titleSize=std::min(28.f,float(w-pw-108)/std::max(1,int(t.name.size()))*1.6f);uiText(tx,y+53,t.name,.02f,.52f,.59f,titleSize);
            uiCash(tx,y+75,40,28);uiText(tx+48,y+99,"$"+std::to_string(sim.baseCash(shopSelection))+" / 20s",.12f,.23f,.28f,21);uiFlag(tx+190,y+75,22,24);uiText(tx+219,y+99,std::to_string(sim.baseFlags(shopSelection)),.12f,.23f,.28f,21);
            std::string caption=shopCaption(t);std::istringstream words(caption);std::string word,line;int lineY=y+153;
            while(words>>word){if(!line.empty()&&line.size()+word.size()>38){uiText(tx,lineY,line,.52f,.19f,.04f,19);line.clear();lineY+=24;}if(!line.empty())line+=" ";line+=word;}
            uiText(tx,lineY,line,.52f,.19f,.04f,19);
            uiText(tx,cy-26,std::to_string(t.w)+" x "+std::to_string(t.h)+" tiles | Build "+std::to_string(sim.buildSeconds(shopSelection))+"s",.12f,.23f,.28f,20);
        }else if(shopCategory==2){uiImage(staffArtwork[staffKind],x+22,y+22,pw-4,ph-4);uiText(tx,y+53,staffNames[staffKind],.02f,.52f,.59f,28);uiText(tx,y+99,"Cleaner for this territory",.12f,.23f,.28f,21);uiText(tx,y+151,"Collect profits and empty bins",.72f,.15f,.05f,18);uiText(tx,y+179,"4 x 4 tiles | 1 cleaner",.12f,.23f,.28f,20);uiImage(0xf2,buttonX,cy-144,128,128);uiText(buttonX,cy-91,locked?"UNLOCK":"BUILD",1,.32f,0,24,128);uiText(buttonX,cy-55,locked?"FREE":"$1800",.12f,.23f,.28f,22,128);uiHit(buttonX,cy-144,128,128,locked?38:75);}else uiText(tx,y+66,"No matching attractions",.02f,.52f,.59f,23);
        if(shopCategory!=2&&(locked||!items.empty())){
            uiImage(0xf2,buttonX,cy-144,128,128);uiText(buttonX,cy-91,locked?"UNLOCK":"BUILD",1,.32f,0,24,128);uiText(buttonX,cy-55,locked?"FREE":"$"+std::to_string(sim.buildPrice(shopSelection)),.12f,.23f,.28f,22,128);uiHit(buttonX,cy-144,128,128,locked?38:35);
        }
        uiPanel(x+12,cy,w-24,120,true);
        for(int category=0;category<3;category++){
            if(shopSlot>=0&&((category==1&&sim.slots[shopSlot].w==16)||(category==2&&sim.slots[shopSlot].w!=4)))continue;
            int ty=cy+7+category*37;
            uiPanel(x+22,ty,144,32,category==shopCategory);
            uiText(x+22,ty+24,category==0?"RIDES":category==1?"SHOPS":"STAFF",.02f,.45f,.52f,22,144);
            uiHit(x+22,ty,144,32,category==0?31:category==1?32:78);
        }
        int cards=shopCategory==2?5:std::min(count,int(items.size())-shopPage*count);
        int step=shopCategory==2?(w-210)/5:cardW;
        for(int j=0;j<cards;j++){
            int index=shopCategory==2?j:items[shopPage*count+j],cx=x+178+j*step;
            uiPanel(cx,cy+9,step-10,102);
            uiModel(shopCategory==2?28+j:sim.types[index].model,cx+5,cy+12,step-20,92);
            if(index==(shopCategory==2?staffKind:shopSelection))gameRect(cx+12,cy+105,step-34,3,.02f,.65f,.72f);
            uiHit(cx,cy+9,step-10,102,shopCategory==2?90+j:1000+index);
        }
        if(pages>1){uiAtlas(910,535,125,160,x+w-52,cy+33,34,50);uiText(x+w-59,cy+103,std::to_string(shopPage+1)+"/"+std::to_string(pages),.03f,.35f,.39f,14,48);uiHit(x+w-58,cy,50,120,34);}
    }
    if(!rideDetails&&!shopOpen&&themePanel<0&&territoryPanel<0){
        for(const auto&entry:themePlots)if(entry.first!=17&&sim.openTerritories.count(entry.first)&&sim.themes[entry.first]==0){const auto&p=entry.second;auto xy=uiWorld((p.x+p.w/2.f)*32,-(p.y+p.h/2.f)*32,24);if(xy.first>280&&xy.first<width-100&&xy.second>100&&xy.second<height-120){uiPanel(xy.first-45,xy.second-18,90,30,true);uiText(xy.first-34,xy.second+3,"THEME",.02f,.4f,.48f,18);}}
    }
    if(themePanel>=0){
        gameButtons.clear();
        int w=480,h=540,x=width-w-16,y=(height-h)/2;
        uiPanel(x,y,w,h);uiText(x+16,y+38,"TERRITORY "+std::to_string(themePanel+1),.02f,.45f,.52f,27);
        uiText(x+16,y+68,"Choose a statue, landscape and paths",.13f,.23f,.27f,18);
        int cw=(w-24)/3,ch=94;
        for(int i=0;i<int(territoryThemes.size());i++){
            int cx=x+14+(i%3)*cw,cy=y+82+(i/3)*ch;const auto&t=territoryThemes[i];
            uiPanel(cx,cy,cw-5,ch-5,i==themeSelection);
            uiModel(t.statue?t.statue:273,cx+5,cy+4,cw-15,ch-35);
            uiText(cx,cy+ch-12,t.name,.02f,.38f,.44f,16,cw-5);uiHit(cx,cy,cw-5,ch-5,60+i);
        }
        uiPanel(x+14,y+h-63,w-140,48,true);uiText(x+14,y+h-32,"APPLY  "+territoryThemes.at(themeSelection).name,.04f,.32f,.38f,20,w-140);uiHit(x+14,y+h-63,w-140,48,72);
        uiPanel(x+w-116,y+h-63,102,48);uiText(x+w-116,y+h-32,"CLOSE",.65f,.18f,.05f,19,102);uiHit(x+w-116,y+h-63,102,48,73);
    }
    if(officePanel){
        gameButtons.clear();int w=448,h=440,x=width-w-16,y=(height-h)/2;
        uiPanel(x,y,w,h);uiText(x+16,y+39,"PARK OFFICE",.02f,.45f,.52f,28);
        uiText(x+16,y+75,"Building style",.13f,.23f,.27f,20);
        const char*styles[]={"CASTLE","CANDY","PALACE"};int cw=(w-24)/3;
        for(int i=0;i<3;i++){
            int cx=x+14+i*cw;uiPanel(cx,y+86,cw-5,123,sim.officeTheme==i);
            uiModel(252+7*i,cx+6,y+91,cw-17,86);uiText(cx,y+196,styles[i],.02f,.38f,.44f,18,cw-5);uiHit(cx,y+86,cw-5,123,80+i);
        }
        uiText(x+16,y+251,"Expand your office",.13f,.23f,.27f,21);
        const char*wings[]={"LEFT WING","RIGHT WING","UPPER FLOOR"};
        for(int i=0;i<3;i++){
            int cx=x+14+i*cw;bool built=sim.officeWings&(1<<i);uiPanel(cx,y+266,cw-5,74,built);
            uiText(cx,y+295,wings[i],.02f,.38f,.44f,17,cw-5);
            uiText(cx,y+323,built?"BUILT":"$2500",.13f,.23f,.27f,18,cw-5);if(!built)uiHit(cx,y+266,cw-5,74,83+i);
        }
        uiPanel(x+14,y+h-64,w-28,46,true);uiText(x+14,y+h-33,"DONE",.03f,.32f,.38f,21,w-28);uiHit(x+14,y+h-64,w-28,46,77);
    }
    if(territoryPanel>=0){
        // Modal hit regions replace underlying HUD actions.
        gameButtons.clear();gameRect(0,0,width,height,0,0,0,.4f);int x=(width-420)/2,y=(height-210)/2;uiPanel(x,y,420,210);
        bool adjacent=false;for(int t:sim.openTerritories)if(std::abs(t%5-territoryPanel%5)+std::abs(t/5-territoryPanel/5)==1)adjacent=true;
        uiText(x+25,y+45,"EXPAND YOUR PARK",.02f,.55f,.6f,27);uiText(x+25,y+86,adjacent?"Open this territory to reveal plots.":"Open a neighbouring territory first.",.15f,.22f,.28f,19);
        uiPanel(x+25,y+124,220,54,true);uiText(x+42,y+159,adjacent?"UNLOCK - FREE":"LOCKED",.02f,.4f,.48f,23);if(adjacent)uiHit(x+25,y+124,220,54,47);
        uiPanel(x+265,y+124,130,54);uiText(x+283,y+159,"CLOSE",.65f,.18f,.05f,23);uiHit(x+265,y+124,130,54,48);
    }
    if(!shopOpen&&!confirmRestart)for(const auto&b:rewardBursts){
        auto p=uiWorld(b.x,b.y,45);float t=float(std::clamp(b.age/.65,0.,1.));float ease=t*t*(3-2*t);
        int tx=b.cash?260:226,ty=b.cash?height-35:70;
        if(rideDetails){tx=b.cash?82:180;ty=48;if(b.cash)uiCash(tx-22,ty-15);else uiFlag(tx-13,ty-14);}
        uiPickupBadge(int(p.first+(tx-p.first)*ease),int(p.second+(ty-p.second)*ease-65*std::sin(t*3.14159f)),b.cash!=0,1-.5f*t);
    }
    if(confirmRestart){gameButtons.clear();int x=(width-430)/2,y=(height-192)/2;gameRect(0,0,width,height,0,0,0,.55f);uiPanel(x,y,430,192);
        uiText(x,y+46,"START A NEW PARK?",.02f,.50f,.54f,26,430);
        uiText(x,y+87,"This replaces your current saved park.",.13f,.25f,.27f,18,430);
        uiPanel(x+18,y+117,185,49,true);uiText(x+18,y+150,"KEEP PLAYING",.12f,.26f,.29f,20,185);uiHit(x+18,y+117,185,49,98);
        uiPanel(x+221,y+117,190,49);uiText(x+221,y+150,"START OVER",.66f,.18f,.05f,20,190);uiHit(x+221,y+117,190,49,97);
    }
    glDisable(GL_BLEND);fontBase=old;
}
