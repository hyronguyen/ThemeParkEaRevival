// Presentation only: camera ownership is separate from park simulation and saves.
struct CameraShot {
    std::array<float,3> center;float radius,zoom,target,yaw,pitch,panX,panY;
};
static CameraShot currentShot(){return {sceneCenter,sceneRadius,zoom,gameZoomTarget,yaw,pitch,panX,panY};}
static float smoothStep(float t){t=std::clamp(t,0.f,1.f);return t*t*(3-2*t);}
static void blendShot(const CameraShot&a,const CameraShot&b,float t){
    t=smoothStep(t);auto mix=[&](float from,float to){return from+(to-from)*t;};
    for(int i=0;i<3;i++)sceneCenter[i]=mix(a.center[i],b.center[i]);
    sceneRadius=mix(a.radius,b.radius);zoom=mix(a.zoom,b.zoom);gameZoomTarget=mix(a.target,b.target);
    yaw=mix(a.yaw,b.yaw);pitch=mix(a.pitch,b.pitch);panX=mix(a.panX,b.panX);panY=mix(a.panY,b.panY);
}
struct GateFilm {int slot=-1;bool premium=false,committed=false;double time=0;CameraShot from{},gate{};} gateFilm;
struct Parade {
    std::vector<int> path;double time=0;bool follow=false;CameraShot from{};
} parade;
static std::map<int,double> readyMarkers;
static bool cameraBusy(){return gateFilm.slot>=0||parade.follow;}
static void beginGateFilm(bool premium){
    if(!rideDetails||selectedRide<0||cameraBusy())return;
    auto&r=sim.rides[selectedRide];
    if(r.gateLevel>=3||(premium?sim.tickets<1:sim.money<sim.upgradePrice(selectedRide)))return;
    const auto&s=sim.slots[r.slot];rideCamera.active=false;
    gateFilm={};gateFilm.slot=r.slot;gateFilm.premium=premium;gateFilm.from=currentShot();
    auto shot=gateFilm.from;shot.center={float((s.x+s.w/2.)*32-16),float(-(s.y+s.h)*32+16),17};
    shot.pitch=36;shot.radius=340;shot.zoom=shot.target=5.5f;shot.panX=shot.panY=0;gateFilm.gate=shot;
}
static void advanceGateFilm(double dt){
    if(gateFilm.slot<0)return;
    auto&f=gateFilm;f.time+=dt;
    if(f.time>=.85&&!f.committed){
        int ride=sim.occupant(f.slot);
        if(ride>=0&&sim.upgrade(ride,f.premium))gameSound("upgrade");
        f.committed=true;
    }
    if(f.time<1.65)blendShot(f.from,f.gate,float(f.time/.85));
    else blendShot(f.gate,f.from,float((f.time-1.65)/.85));
    if(f.time>=2.5)f.slot=-1;
}
static void beginParade(){
    parade.path={sim.spawn};unsigned seed=sim.rank()*137u+int(sim.time);
    for(int n=0;n<160;n++){
        int at=parade.path.back(),previous=n?parade.path[n-1]:-1;
        std::vector<int> choices;for(int next:sim.edges[at])
            if(next!=previous&&sim.openTerritories.count(sim.tileTerritory(next)))choices.push_back(next);
        if(choices.empty()){if(previous<0)break;choices.push_back(previous);}
        seed=seed*1664525u+1013904223u;int next=choices[(seed>>8)%choices.size()];
        if(n<6)for(int tile:choices)if(sim.tiles[tile].y<sim.tiles[next].y)next=tile;
        parade.path.push_back(next);
    }
    if(parade.path.size()<2){parade.path.clear();return;}
    parade.time=0;parade.follow=true;parade.from=currentShot();gameSound("parade");
}
static std::array<float,3> paradePosition(double progress){
    progress=std::clamp(progress,0.,double(parade.path.size()-1)-.001);int n=int(progress);
    const auto&a=sim.tiles[parade.path[n]],b=sim.tiles[parade.path[n+1]];float t=float(progress-n);
    return {32*(a.x+.5f+(b.x-a.x)*t),-32*(a.y+.5f+(b.y-a.y)*t),float(std::atan2(-(b.y-a.y),b.x-a.x)*57.29578)+90};
}
static void advanceParade(double dt){
    if(dt>0&&parade.path.empty()&&sim.rank()>sim.celebratedRank&&!cameraBusy()&&!rideDetails&&!rideCamera.active&&!shopOpen&&!confirmRestart&&!settingsOpen&&territoryPanel<0&&themePanel<0&&!officePanel){
        beginParade();
        if(parade.follow)++sim.celebratedRank;
    }
    if(parade.path.empty())return;
    parade.time+=dt;
    if(parade.follow){
        auto pos=paradePosition(std::max(0.,parade.time*1.8-1.8));auto shot=parade.from;
        shot.center={pos[0],pos[1],8};shot.zoom=shot.target=3.8f;shot.radius=410;shot.pitch=38;shot.panX=shot.panY=0;
        float weight=float(std::min(parade.time/.65,(5-parade.time)/.65));
        blendShot(parade.from,shot,weight);
        if(parade.time>=5)parade.follow=false;
    }
    if(parade.time>=65)parade.path.clear();
}
