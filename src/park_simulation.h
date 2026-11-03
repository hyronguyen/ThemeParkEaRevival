// Offline playtest rules, explicitly provisional; original resources stay unchanged.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <queue>
#include <set>
#include <iomanip>
#include "original_economy.h"
namespace playtest {
struct Tile {int x=0,y=0;bool operator<(const Tile&o)const{return std::tie(x,y)<std::tie(o.x,o.y);}bool operator==(const Tile&o)const{return x==o.x&&y==o.y;}};
struct Slot {int id,x,y,w,h;};
struct Type {int model,w,h,capacity;double duration;std::string name;int price=0,buildSeconds=20;bool shop=false;};
struct Ride {int slot,type,rotation=0,served=0,gateLevel=0;bool open=true;double timer=0,boarding=0;std::vector<int> queue,riders;
    bool running=false;double buildRemaining=0;int profit=0,rewardFlags=0;double incomeClock=0,challenge=-1,happiness=50,goodTime=0,tapSpeed=0,animationTime=0;};
enum class State {Walking,Queued,Riding,Leaving,Idle,OnBus,WaitingBus};
struct Guest {int tile=0,target=-1,visits=0,seat=-1;State state=State::Idle;std::vector<int> route;size_t step=0;double progress=0,wait=0,walkTime=0;};
struct TrashBin {int x=0,y=0,territory=0;double remaining=60;};
struct Staff {int slot=0,tile=0,bin=-1,kind=1,collectSlot=-1;bool gathering=false;double lootX=0,lootY=0;std::vector<int> route;size_t step=0;double progress=0,wait=0,walkTime=0;};
struct Loot {double x=0,y=0,dx=0,dy=0,age=0;int cash=0,flags=0;};
struct Park {
    std::vector<Loot> loot, pickedItems;
    std::set<int> spills;
    bool cleanSpill(int tile){return spills.erase(tile)!=0;}
    bool sellStaff(int slot){
        int index=staffAt(slot);if(index<0)return false;
        staff.erase(staff.begin()+index);money=std::min(1000000000,money+900);return true;
    }
    void dropReward(double x,double y,int cash,int xp){for(int category=0;category<2;category++){int value=category?xp:cash,count=std::min(value,category?3:5);for(int i=0;i<count;i++){int amount=value/count+(i<value%count);double angle=(i+(category?.45:0))*6.283185307/std::max(1,count);Loot item;item.x=x;item.y=y;item.dx=std::cos(angle)*(category?72:45);item.dy=std::sin(angle)*(category?72:45);item.cash=category?0:amount;item.flags=category?amount:0;loot.push_back(item);}}}
    bool releaseProfit(int index,bool countStreak=true){if(index<0||index>=int(rides.size()))return false;auto&r=rides[index];if(!r.profit&&!r.rewardFlags)return false;const auto&s=slots[r.slot];size_t first=loot.size();dropReward((s.x+s.w/2.)*32,-(s.y+s.h/2.)*32,r.profit*(100+bonus())/100,r.rewardFlags);for(size_t i=first;i<loot.size();i++){loot[i].dx*=.75;loot[i].dy=loot[i].dy*.55-s.h*11.;}r.profit=r.rewardFlags=0;if(countStreak)streak=std::min(10,streak+1);return true;}
    bool pickup(int index){if(index<0||index>=int(loot.size()))return false;if(pickedItems.size()<64)pickedItems.push_back(loot[index]);money=std::min(1000000000,money+loot[index].cash);addFlags(loot[index].flags);loot.erase(loot.begin()+index);return true;}
    std::vector<Staff> staff;
    int officeTheme=-1,officeWings=0;
    int tileTerritory(int tile)const{const auto&t=tiles.at(tile);return std::clamp((t.y-1)/36,0,3)*5+std::clamp((t.x-1)/36,0,4);}
    int staffAt(int slot)const{for(size_t i=0;i<staff.size();i++)if(staff[i].slot==slot)return int(i);return -1;}
    std::vector<int> staffRoute(int from,int to,int area)const{
        if(from<0||to<0||tileTerritory(from)!=area||tileTerritory(to)!=area)return {};
        std::vector<int> previous(tiles.size(),-1);std::queue<int> q;q.push(from);previous[from]=from;
        while(!q.empty()&&previous[to]<0){int at=q.front();q.pop();for(int next:edges[at])if(previous[next]<0&&tileTerritory(next)==area){previous[next]=at;q.push(next);}}
        if(previous[to]<0)return {};std::vector<int> result;for(int at=to;;at=previous[at]){result.push_back(at);if(at==from)break;}std::reverse(result.begin(),result.end());return result;
    }
    bool buyStaff(int slot,int kind=1){if(kind<0||kind>4||slot<0||slot>=int(slots.size())||slots[slot].w!=4||slots[slot].h!=4||occupant(slot)>=0||staffAt(slot)>=0||lockedSlots.count(slot)||!territoryOpen(slot)||money<1800)return false;
        int gate=entrance(slot);if(gate<0||tileTerritory(gate)!=territory(slot))return false;Staff s;s.slot=slot;s.tile=gate;s.kind=kind;staff.push_back(s);money-=1800;return true;}
    bool extendOffice(int wing){if(wing<0||wing>2||(officeWings&(1<<wing))||money<2500)return false;money-=2500;if(officeTheme<0)officeTheme=0;officeWings|=1<<wing;return true;}
    void tickStaff(double dt){for(auto&s:staff){int area=territory(s.slot);
        if(s.step+1<s.route.size()){s.progress+=dt*1.5;s.walkTime+=dt;while(s.progress>=1&&s.step+1<s.route.size()){s.progress-=1;s.tile=s.route[++s.step];}if(s.step+1<s.route.size())continue;s.progress=0;s.route.clear();s.step=0;s.wait=s.bin>=0?3:1;}
        if(s.wait>0){s.wait=std::max(0.,s.wait-dt);if(s.wait>0)continue;if(s.bin>=0){cleanBin(s.bin);s.bin=-1;}if(s.collectSlot>=0){int ride=occupant(s.collectSlot);if(ride>=0&&collect(ride)>0)streak=std::min(10,streak+1);s.collectSlot=-1;}
            if(s.gathering){
                for(size_t i=loot.size();i-->0;)if(loot[i].x==s.lootX&&loot[i].y==s.lootY)pickup(int(i));
                s.gathering=false;
            }
        }
        int destination=-1;double best=1e30;s.bin=-1;
        for(size_t i=0;i<bins.size();i++)if(bins[i].territory==area&&bins[i].remaining<=0){int nearestTile=-1;double distance=1e30;for(size_t j=0;j<tiles.size();j++)if(tileTerritory(int(j))==area){double d=std::hypot(double(tiles[j].x-bins[i].x),double(tiles[j].y-bins[i].y));if(d<distance){distance=d;nearestTile=int(j);}}auto path=staffRoute(s.tile,nearestTile,area);if(!path.empty()&&path.size()<best){best=double(path.size());destination=nearestTile;s.bin=int(i);}}
        if(destination<0)for(const auto&r:rides)if(territory(r.slot)==area&&(r.profit>0||r.rewardFlags>0)){int gate=entrance(r.slot);auto path=staffRoute(s.tile,gate,area);if(!path.empty()&&path.size()<best){best=double(path.size());destination=gate;s.collectSlot=r.slot;}}
        if(destination<0){
            for(const auto&item:loot){
                int x=int(item.x/32),y=int(-item.y/32);
                if(std::clamp((y-1)/36,0,3)*5+std::clamp((x-1)/36,0,4)!=area)continue;
                int nearestPath=-1;double distance=1e30;
                for(size_t i=0;i<tiles.size();i++)if(tileTerritory(int(i))==area){
                    double dx=tiles[i].x-x,dy=tiles[i].y-y;
                    if(dx*dx+dy*dy<distance){nearestPath=int(i);distance=dx*dx+dy*dy;}
                }
                if(staffRoute(s.tile,nearestPath,area).empty())continue;
                destination=nearestPath;s.gathering=true;s.lootX=item.x;s.lootY=item.y;break;
            }
        }
        if(destination<0){std::vector<int> candidates;for(size_t j=0;j<tiles.size();j++)if(tileTerritory(int(j))==area&&int(j)!=s.tile)candidates.push_back(int(j));if(!candidates.empty())destination=candidates[random()%candidates.size()];}
        s.route=staffRoute(s.tile,destination,area);s.step=0;s.progress=0;if(s.route.size()<=1){s.route.clear();s.wait=s.bin>=0?3:2;}
    }}
    std::vector<TrashBin> bins;
    double nextBinInterval(){return 240+random()%361;}
    bool cleanBin(int i,bool drops=false){if(i<0||i>=int(bins.size())||bins[i].remaining>0||(enforceTerritories&&!openTerritories.count(bins[i].territory)))return false;bins[i].remaining=nextBinInterval();if(drops)dropReward((bins[i].x+.5)*32,-(bins[i].y+.5)*32,0,2);else addFlags(2);return true;}
    bool playing(const Ride&r)const{return r.buildRemaining<=0&&(r.running||r.challenge>=0);}
    int visualPassengers(const Ride&r)const{return r.challenge>=0||(r.running&&!r.riders.empty())?types[r.type].capacity:int(r.riders.size());}
    std::vector<Tile> tiles;std::map<Tile,int> tileIndex;std::vector<std::vector<int>> edges;
    std::vector<Slot> slots;std::vector<Type> types;std::vector<Ride> rides;std::vector<Guest> guests;
    std::set<int> lockedSlots,openTerritories;
    bool enforceTerritories=false,adaptiveCrowd=false;
    int crowdTarget()const{
        int demand=6+int(openTerritories.size())*3;
        for(const auto&r:rides)if(r.open&&r.buildRemaining<=0)demand+=types[r.type].shop?1:2+std::min(4,types[r.type].capacity/4);
        return std::min(120,demand);
    }
    int baseCash(int type)const{return 30+std::max(0,types[type].w-4)*5;}
    int baseFlags(int type)const{return types[type].w>=16?5:types[type].w>=6?3:2;}
    std::array<int,20> themes{};
    bool setTheme(int territory,int theme){if(territory==17||territory<0||territory>=20||theme<0||theme>11||(enforceTerritories&&!openTerritories.count(territory)))return false;themes[territory]=theme;return true;}
    int territory(int slot)const{const auto&s=slots.at(slot);int x=int((s.x+s.w/2.-1)/36),y=int((s.y+s.h/2.-1)/36);return std::clamp(y,0,3)*5+std::clamp(x,0,4);}
    bool territoryOpen(int slot)const{return !enforceTerritories||openTerritories.count(territory(slot));}
    bool expand(int t){if(t<0||t>=20||openTerritories.count(t))return false;for(int o:openTerritories)if(std::abs(o%5-t%5)+std::abs(o/5-t/5)==1){openTerritories.insert(t);return true;}return false;}
    int money=125000,tickets=10,flags=0,streak=0;
    // Preserve total earned flags in saves; derive level/progress from bundled defaults.
    int rank()const{int level=1,left=flags;while(level<int(originalXP.size())&&left>=originalXP[level-1])left-=originalXP[level++-1];return level;}
    int rankProgress()const{int left=flags;for(int i=1;i<rank();i++)left-=originalXP[i-1];return left;}
    int rankTarget()const{return originalXP[rank()-1];}
    bool maxRank()const{return rank()==int(originalXP.size());}
    int celebratedRank=1;
    void addFlags(int amount){int oldRank=rank();flags=int(std::min(1000000000LL,static_cast<long long>(flags)+std::max(0,amount)));tickets=std::min(1000000,tickets+rank()-oldRank);}
    int bonus()const{return std::min(50,streak*5);}
    int buildPrice(int type)const{const auto&t=types.at(type);return t.price>0?t.price:t.w*t.h*20;}
    int buildSeconds(int type)const{return types.at(type).buildSeconds;}
    void setLocalBalance(){for(auto&t:types){int tier=std::max(t.w,t.h),step=t.model%100;bool shop=t.shop;
        t.price=shop?1200+step*120:tier<=4?7200+step*180:tier<=6?9000+step*400:35000+step*1000;
        t.buildSeconds=shop?10+(step%5)*5:tier<=4?15+(step%5)*5:tier<=6?20+(step%5)*10:60+(step%5)*15;
        if(t.model==404){t.price=2600;t.buildSeconds=3;}if(t.model==449){t.price=6300;t.buildSeconds=10;}if(t.model==476){t.price=6800;t.buildSeconds=15;}
    }}
    int upgradePrice(int index)const{return 100*(rides.at(index).gateLevel+1);}
    bool upgrade(int index,bool premium=false){auto&r=rides.at(index);if(r.gateLevel>=3)return false;int price=upgradePrice(index);if(premium){if(tickets<1)return false;--tickets;}else{if(money<price)return false;money-=price;}++r.gateLevel;return true;}
    int collect(int index){auto&r=rides.at(index);int cash=r.profit*(100+bonus())/100;money=std::min(1000000000,money+cash);addFlags(r.rewardFlags);r.profit=r.rewardFlags=0;return cash;}
    int freePathTile(int origin,const Guest* ignore=nullptr)const{
        std::queue<int> pending;std::set<int> seen;pending.push(origin);seen.insert(origin);
        while(!pending.empty()){int tile=pending.front();pending.pop();bool occupied=false;
            for(const auto&other:guests)if(&other!=ignore&&other.state!=State::Riding&&other.state!=State::OnBus){auto p=position(other);if(std::hypot(p.first/32-(tiles[tile].x+.5),-p.second/32-(tiles[tile].y+.5))<.7){occupied=true;break;}}
            if(!occupied)return tile;for(int next:edges[tile])if(seen.insert(next).second)pending.push(next);
        }return origin;
    }
    void finishRide(Ride&r,bool completed){
        for(int id:r.riders){auto&g=guests[id];g.tile=freePathTile(entrance(r.slot),&g);g.seat=-1;g.wait=1;g.route.clear();g.step=0;g.progress=0;
            if(completed){g.visits++;r.served++;served++;}
            if(g.visits>=3)send(g,spawn,State::Leaving);else{g.state=State::Idle;g.target=-1;}}
        r.riders.clear();r.timer=r.boarding=0;r.running=false;
    }
    void cancelChallenge(int index){auto&r=rides.at(index);if(r.challenge<0)return;r.challenge=-2;r.tapSpeed=0;finishRide(r,false);}
    bool tap(int index){auto&r=rides.at(index);if(!r.open||r.buildRemaining>0||r.challenge==-2||types[r.type].capacity<=0)return false;
        // Unoccupied minigame seats are render-only guests, never park visitors.
        if(r.challenge<0){r.challenge=0;r.happiness=50;r.goodTime=0;r.running=true;r.timer=20;r.boarding=0;for(int id:r.riders)guests[id].wait=20;}
        r.happiness=std::min(100.,r.happiness+8);r.tapSpeed=std::min(6.,r.tapSpeed+.8);return true;
    }
    void sell(int index){auto&r=rides.at(index);int gate=entrance(r.slot);money+=buildPrice(r.type)/2+50*r.gateLevel;
        for(auto&g:guests){if(g.target==index){g.target=-1;g.tile=freePathTile(gate,&g);g.state=State::Idle;g.route.clear();g.step=0;g.progress=0;g.wait=.5;}else if(g.target>index)--g.target;}
        rides.erase(rides.begin()+index);}
    bool buses=false;int spawn=0,served=0;double time=0;bool paused=false;int speed=1;unsigned seed=12345;
    unsigned random(){seed=1664525u*seed+1013904223u;return seed;}
    void graph(const std::set<Tile>&points){
        entranceCache.clear();components.clear();
        tiles.assign(points.begin(),points.end());tileIndex.clear();edges.assign(tiles.size(),{});
        for(size_t i=0;i<tiles.size();i++)tileIndex[tiles[i]]=int(i);
        for(size_t i=0;i<tiles.size();i++)for(auto d:std::array<Tile,4>{{{-1,0},{1,0},{0,-1},{0,1}}}){
            auto it=tileIndex.find({tiles[i].x+d.x,tiles[i].y+d.y});if(it!=tileIndex.end())edges[i].push_back(it->second);}
    }
    std::vector<int> route(int from,int to)const{
        if(from<0||to<0||from>=int(tiles.size())||to>=int(tiles.size()))return {};
        std::vector<int> previous(tiles.size(),-1);std::queue<int> q;q.push(from);previous[from]=from;
        while(!q.empty()&&previous[to]<0){int at=q.front();q.pop();for(int next:edges[at])if(previous[next]<0){previous[next]=at;q.push(next);}}
        if(previous[to]<0)return {};std::vector<int> result;for(int at=to;;at=previous[at]){result.push_back(at);if(at==from)break;}
        std::reverse(result.begin(),result.end());return result;
    }
    mutable std::map<int,std::pair<size_t,int>> entranceCache;
    mutable std::vector<int> components;
    bool reachable(int from,int to)const{
        if(from<0||to<0||from>=int(tiles.size())||to>=int(tiles.size()))return false;
        if(components.size()!=tiles.size()){components.assign(tiles.size(),-1);for(size_t i=0;i<tiles.size();i++)if(components[i]<0){std::queue<int> q;q.push(int(i));components[i]=int(i);while(!q.empty()){int at=q.front();q.pop();for(int next:edges[at])if(components[next]<0){components[next]=int(i);q.push(next);}}}}
        return components[from]==components[to];
    }
    int entrance(int slot)const{
        auto cached=entranceCache.find(slot);if(cached!=entranceCache.end()&&cached->second.first==tiles.size())return cached->second.second;
        const auto&s=slots.at(slot);int best=-1;double distance=1e30;
        for(size_t i=0;i<tiles.size();i++){double x=tiles[i].x+.5,y=tiles[i].y+.5;
            double dx=std::max({double(s.x)-x,0.,x-double(s.x+s.w)}),dy=std::max({double(s.y)-y,0.,y-double(s.y+s.h)});
            if(dx+dy>1.51)continue;double score=std::pow(x-(s.x+s.w/2.-.5),2)+std::pow(y-(s.y+s.h+.5),2); // Fence opening is half a tile left of center.
            if(score<distance){distance=score;best=int(i);}}
        entranceCache[slot]={tiles.size(),best};return best;
    }
    int occupant(int slot)const{for(size_t i=0;i<rides.size();i++)if(rides[i].slot==slot)return int(i);return -1;}
    bool canBuild(int slot,int type,int rotation=0)const{
        if(slot<0||slot>=int(slots.size())||type<0||type>=int(types.size())||occupant(slot)>=0||lockedSlots.count(slot)||!territoryOpen(slot))return false;
        const auto&s=slots[slot];const auto&t=types[type];if(staffAt(slot)>=0||(s.w==16&&s.h==16&&t.shop))return false;int w=(rotation%180)?t.h:t.w,h=(rotation%180)?t.w:t.h;
        int gate=entrance(slot);return w==s.w&&h==s.h&&gate>=0&&!route(spawn,gate).empty();
    }
    int build(int slot,int type,int rotation=0){if(!canBuild(slot,type,rotation))return -1;Ride r;r.slot=slot;r.type=type;r.rotation=rotation;rides.push_back(r);return int(rides.size()-1);}
    int purchase(int slot,int type,int rotation=0){if(!canBuild(slot,type,rotation)||money<buildPrice(type))return -1;int result=build(slot,type,rotation);money-=buildPrice(type);rides[result].buildRemaining=buildSeconds(type);return result;}
    void addGuest(){if(guests.size()>=160||tiles.empty())return;Guest g;g.tile=buses?spawn:freePathTile(spawn);g.wait=double(guests.size()%8)*.2;if(buses){g.state=State::OnBus;g.wait=40-std::fmod(time,40.)+10+double(guests.size()%8)*.4;if(time==0)g.wait=10+double(guests.size())*.25;}guests.push_back(g);}
    void send(Guest&g,int destination,State state){g.route=route(g.tile,destination);g.step=0;g.progress=0;g.state=state;if(g.route.empty()){g.state=State::Idle;g.wait=2;}}
    void choose(Guest&g){
        std::vector<int> candidates;for(size_t i=0;i<rides.size();i++)if(rides[i].open&&rides[i].buildRemaining<=0&&rides[i].challenge<0&&rides[i].riders.size()<size_t(types[rides[i].type].capacity)&&reachable(g.tile,entrance(rides[i].slot)))candidates.push_back(int(i));
        if(candidates.empty()){g.target=-1;g.state=State::Idle;g.wait=2;return;}
        g.target=candidates[random()%candidates.size()];send(g,entrance(rides[g.target].slot),State::Walking);
    }
    void close(int index){
        Ride&r=rides.at(index);r.open=!r.open;if(r.open)return;
        for(int id:r.queue){auto&g=guests[id];g.target=-1;g.state=State::Idle;g.wait=.5;}r.queue.clear();
        // Riders finish safely; new admissions stop immediately.
    }
    void tick(double dt){
        if(paused)return;double previousTime=time;time+=dt;if(adaptiveCrowd&&guests.size()<160&&int(time/9)>int(previousTime/9)&&guests.size()<size_t(crowdTarget()))addGuest();bool admit=!adaptiveCrowd||int(time/3)>int(previousTime/3);tickStaff(dt);for(auto&item:loot)item.age=std::min(10.,item.age+dt);
        if(int(time)>int(previousTime)){
            bool spillDue=int(time/90)>int(previousTime/90)&&spills.size()<12;
            for(size_t n=0;n<guests.size();n++){
                const auto&guest=guests[(n+size_t(time))%guests.size()];
                if(guest.state!=State::Walking&&guest.state!=State::Leaving)continue;
                if(enforceTerritories&&!openTerritories.count(tileTerritory(guest.tile)))continue;
                if(spillDue){spills.insert(guest.tile);spillDue=false;}
                auto pos=position(guest);
                for(size_t j=loot.size();j-->0;){
                    const auto&item=loot[j];
                    if(item.age>=4&&std::hypot(item.x+item.dx-pos.first,item.y+item.dy-pos.second)<56)pickup(int(j));
                }
            }
        }
        for(size_t i=0;i<guests.size();i++){Guest&g=guests[i];if(g.state!=State::Riding)g.wait=std::max(0.,g.wait-dt);
            if(g.state==State::OnBus){if(g.wait==0&&admit){if(adaptiveCrowd)admit=false;g.state=State::Idle;g.tile=freePathTile(spawn,&g);g.visits=0;g.target=-1;}continue;}
            if(g.state==State::WaitingBus){double phase=std::fmod(time,40.);if(phase>=18&&phase<25){g.state=State::OnBus;g.wait=40-phase+10+double(i%8)*.4;}continue;}
            if(g.state==State::Idle){if(g.wait==0)choose(g);continue;}
            if(g.state==State::Walking&&(!rides[g.target].open||rides[g.target].buildRemaining>0||rides[g.target].challenge>=0||rides[g.target].riders.size()>=size_t(types[rides[g.target].type].capacity))){
                // Abort from a tile, never from the middle of an edge.
                if(g.progress==0){g.state=State::Idle;g.wait=.5;continue;}}
            if(g.state!=State::Walking&&g.state!=State::Leaving)continue;
            // Keep a following distance in the same lane; opposing traffic may pass.
            double advance=dt*1.4;
            if(g.step+1<g.route.size()){
                auto a=tiles[g.tile],b=tiles[g.route[g.step+1]];double dx=b.x-a.x,dy=b.y-a.y;
                double x=a.x+.5+dx*g.progress,y=a.y+.5+dy*g.progress;
                for(size_t j=0;j<guests.size();j++){if(i==j)continue;const auto&o=guests[j];
                    if(o.state!=State::Walking&&o.state!=State::Leaving)continue;if(o.step+1>=o.route.size())continue;
                    auto c=tiles[o.tile],d=tiles[o.route[o.step+1]];double ox=d.x-c.x,oy=d.y-c.y;
                    if(dx*ox+dy*oy<.5)continue;
                    double rx=c.x+.5+ox*o.progress-x,ry=c.y+.5+oy*o.progress-y;
                    double ahead=rx*dx+ry*dy,side=std::abs(rx*dy-ry*dx);
                    if(side<.3&&(ahead>1e-6||(std::abs(ahead)<1e-6&&j<i)))advance=std::min(advance,std::max(0.,ahead-.48));
                }
            }
            if(advance>0)g.walkTime+=advance/1.4;g.progress+=advance;
            while(g.step+1<g.route.size()&&g.progress>=1){g.progress-=1;g.tile=g.route[++g.step];}
            if(g.step+1>=g.route.size()){
                g.progress=0;g.route.clear();g.step=0;
                if(g.state==State::Leaving){g.state=buses?State::WaitingBus:State::Idle;g.wait=2;g.visits=0;g.target=-1;}
                else if(rides[g.target].open&&rides[g.target].buildRemaining<=0&&rides[g.target].challenge<0&&rides[g.target].riders.size()<size_t(types[rides[g.target].type].capacity)&&random()%100<90){
                    auto&r=rides[g.target];g.state=State::Riding;g.wait=20;g.seat=0;while(std::any_of(r.riders.begin(),r.riders.end(),[&](int id){return guests[id].seat==g.seat;}))++g.seat;r.riders.push_back(int(i));
                    const auto& plot=slots[r.slot];
                    for(size_t j=loot.size();j-->0;)if(loot[j].age>=4&&loot[j].x==(plot.x+plot.w/2.)*32&&loot[j].y==-(plot.y+plot.h/2.)*32)pickup(int(j));
                }
                else{g.state=State::Idle;g.wait=.5;}
            }
        }
        for(auto&bin:bins)if(!enforceTerritories||openTerritories.count(bin.territory))bin.remaining=std::max(0.,bin.remaining-dt);
        for(auto&r:rides){
            if(r.running&&r.riders.empty()&&r.challenge<0){r.running=false;r.timer=0;}
            if(r.buildRemaining>0){r.buildRemaining=std::max(0.,r.buildRemaining-dt);continue;}
            if(r.open){r.incomeClock+=dt;while(r.incomeClock>=20){r.incomeClock-=20;r.profit=std::min(1000000,r.profit+baseCash(r.type)+10*r.gateLevel);r.rewardFlags=std::min(10000,r.rewardFlags+baseFlags(r.type)+r.gateLevel);}}
            r.tapSpeed=std::max(0.,r.tapSpeed-dt*1.2);
            for(int id:r.queue){guests[id].state=State::Idle;guests[id].wait=.5;guests[id].target=-1;}r.queue.clear();
            if(r.challenge>=0){r.animationTime+=dt*(1+r.tapSpeed);r.challenge+=dt;r.timer=std::max(0.,20-r.challenge);r.happiness=std::max(0.,r.happiness-dt*14);
                if(r.happiness>=30&&r.happiness<=80)r.goodTime+=dt;
                for(int id:r.riders)guests[id].wait=r.timer;
                if(r.challenge>=20){bool happy=r.happiness>=30&&r.happiness<=80&&r.goodTime>=10;r.profit+=happy?80:30;r.rewardFlags+=happy?5:2;streak=happy?std::min(100,streak+1):0;r.challenge=-2;r.tapSpeed=0;finishRide(r,true);}continue;
            }
            if(r.running){r.animationTime+=dt;r.timer=std::max(0.,r.timer-dt);for(int id:r.riders)guests[id].wait=r.timer;if(r.timer<=1e-8)finishRide(r,true);}
            else if(!r.riders.empty()){
                r.boarding+=dt;
                if(!r.riders.empty()){r.running=true;r.timer=20;r.boarding=0;for(int id:r.riders)guests[id].wait=20;}
            }
        }
    }

    std::pair<double,double> position(const Guest&g)const{
        const Tile&a=tiles.at(g.tile);double x=a.x+.5,y=a.y+.5;
        if((g.state==State::Walking||g.state==State::Leaving)&&g.step+1<g.route.size()){
            const auto&b=tiles[g.route[g.step+1]];double dx=b.x-a.x,dy=b.y-a.y;x+=dx*g.progress;y+=dy*g.progress;
            // Right-hand lanes meet smoothly at bends, without changing route cells.
            double sx=dx,sy=dy,ex=dx,ey=dy;
            if(g.step>0){auto prev=tiles[g.route[g.step-1]];sx=(dx+a.x-prev.x)*.5;sy=(dy+a.y-prev.y)*.5;}
            if(g.step+2<g.route.size()){auto next=tiles[g.route[g.step+2]];ex=(dx+next.x-b.x)*.5;ey=(dy+next.y-b.y)*.5;}
            double t=g.progress*g.progress*(3-2*g.progress);x-=.13*(sy+(ey-sy)*t);y+=.13*(sx+(ex-sx)*t);}return {x*32,-y*32};
    }
    void validate()const{
        auto check=[](bool b){if(!b)throw std::runtime_error("Invalid playtest state");};
        check(spawn>=0&&spawn<int(tiles.size())&&rides.size()<=slots.size()&&guests.size()<=256&&std::isfinite(time)&&time>=0&&time<1e9&&served>=0);
        check(money>=0&&money<=1000000000&&tickets>=0&&tickets<=1000000&&flags>=0&&flags<=1000000000&&streak>=0&&streak<=100);
        for(const auto&bin:bins)check(bin.territory>=0&&bin.territory<20&&std::isfinite(bin.remaining)&&bin.remaining>=0&&bin.remaining<=600);
        for(int theme:themes)check(theme>=0&&theme<=11);
        for(int t:openTerritories)check(t>=0&&t<20);check(!enforceTerritories||!openTerritories.empty());
        for(int slot:lockedSlots)check(slot>=0&&slot<int(slots.size())&&occupant(slot)<0);
        std::set<int> occupied;std::vector<int> membership(guests.size(),0);
        check(officeTheme>=-1&&officeTheme<=2&&officeWings>=0&&officeWings<=7);
        for(const auto&s:staff){check(s.kind>=0&&s.kind<5);check(s.slot>=0&&s.slot<int(slots.size())&&slots[s.slot].w==4&&slots[s.slot].h==4&&occupied.insert(s.slot).second&&territoryOpen(s.slot)&&!lockedSlots.count(s.slot));check(s.tile>=0&&s.tile<int(tiles.size())&&tileTerritory(s.tile)==territory(s.slot));}
        for(size_t i=0;i<rides.size();i++){const auto&r=rides[i];check(r.slot>=0&&r.slot<int(slots.size())&&r.type>=0&&r.type<int(types.size())&&occupied.insert(r.slot).second);
            check(territoryOpen(r.slot));check(std::isfinite(r.buildRemaining)&&r.buildRemaining>=0&&r.buildRemaining<=600);check(r.gateLevel>=0&&r.gateLevel<=3);check(r.rotation>=0&&r.rotation<360&&r.rotation%90==0&&std::isfinite(r.timer)&&r.timer>=0&&r.timer<=std::max(30.,types[r.type].duration)&&std::isfinite(r.boarding)&&r.boarding>=0&&r.served>=0);
            check(r.profit>=0&&r.profit<=2000000&&r.rewardFlags>=0&&r.rewardFlags<=20000&&std::isfinite(r.incomeClock)&&r.incomeClock>=0&&r.incomeClock<20&&std::isfinite(r.challenge)&&r.challenge>=-2&&r.challenge<20&&std::isfinite(r.happiness)&&r.happiness>=0&&r.happiness<=100&&std::isfinite(r.goodTime)&&r.goodTime>=0&&r.goodTime<21&&std::isfinite(r.tapSpeed)&&r.tapSpeed>=0&&r.tapSpeed<=6&&std::isfinite(r.animationTime)&&r.animationTime>=0);
            const auto&s=slots[r.slot];const auto&t=types[r.type];check((r.rotation%180?t.h:t.w)<=s.w&&(r.rotation%180?t.w:t.h)<=s.h&&!route(spawn,entrance(r.slot)).empty());
            check(r.riders.size()<=size_t(types[r.type].capacity)&&r.queue.size()<=24&&(r.timer==0||!r.riders.empty()||r.challenge>=0));
            for(int mode=0;mode<2;mode++)for(int id:mode?r.riders:r.queue){check(id>=0&&id<int(guests.size()));check(++membership[id]==1&&guests[id].target==int(i)&&guests[id].state==(mode?State::Riding:State::Queued));}}
        for(size_t i=0;i<guests.size();i++){const auto&g=guests[i];check(g.tile>=0&&g.tile<int(tiles.size())&&int(g.state)>=0&&int(g.state)<=6&&g.target>=-1&&g.target<int(rides.size())&&g.visits>=0);
            check(std::isfinite(g.progress)&&g.progress>=0&&g.progress<1&&std::isfinite(g.wait)&&g.wait>=0&&g.wait<1e6&&std::isfinite(g.walkTime)&&g.walkTime>=0&&g.walkTime<1e9);
            bool moving=g.state==State::Walking||g.state==State::Leaving;check((g.state!=State::Walking&&g.state!=State::Queued&&g.state!=State::Riding)||g.target>=0);
            check((g.state==State::Queued||g.state==State::Riding)?membership[i]==1:membership[i]==0);
            if(moving){check(!g.route.empty()&&g.step<g.route.size()&&g.route[g.step]==g.tile);for(size_t j=0;j<g.route.size();j++){check(g.route[j]>=0&&g.route[j]<int(tiles.size()));if(j)check(std::find(edges[g.route[j-1]].begin(),edges[g.route[j-1]].end(),g.route[j])!=edges[g.route[j-1]].end());}}
        }
    }
    void write(std::ostream&out)const{
        validate();out<<std::setprecision(17)<<"TPPLAY13 "<<time<<' '<<seed<<' '<<served<<' '<<spawn<<' '<<rides.size()<<' '<<guests.size()<<'\n';
        out<<money<<' '<<tickets<<' '<<flags<<' '<<streak<<'\n';
        out<<lockedSlots.size();for(int slot:lockedSlots)out<<' '<<slot;out<<'\n';
        for(const auto&r:rides){out<<r.slot<<' '<<r.type<<' '<<r.rotation<<' '<<r.open<<' '<<r.served<<' '<<r.timer<<' '<<r.boarding<<' '<<r.queue.size();for(int g:r.queue)out<<' '<<g;out<<' '<<r.riders.size();for(int g:r.riders)out<<' '<<g;out<<' '<<r.gateLevel<<' '<<r.profit<<' '<<r.rewardFlags<<' '<<r.incomeClock<<' '<<r.challenge<<' '<<r.happiness<<' '<<r.goodTime<<' '<<r.tapSpeed<<' '<<r.animationTime<<'\n';}
        for(const auto&g:guests){out<<g.tile<<' '<<g.target<<' '<<g.visits<<' '<<int(g.state)<<' '<<g.step<<' '<<g.progress<<' '<<g.wait<<' '<<g.walkTime<<' '<<g.route.size();for(int t:g.route)out<<' '<<t;out<<'\n';}
        out<<openTerritories.size();for(int t:openTerritories)out<<' '<<t;out<<'\n';
        for(int theme:themes)out<<theme<<' ';out<<'\n';
        for(const auto&r:rides)out<<r.buildRemaining<<' '<<r.running<<'\n';
        for(const auto&g:guests)out<<g.seat<<' ';out<<'\n';
        out<<bins.size()<<'\n';for(const auto&bin:bins)out<<bin.remaining<<' ';out<<'\n';
        out<<officeTheme<<' '<<officeWings<<' '<<staff.size()<<'\n';for(const auto&s:staff)out<<s.slot<<' '<<s.kind<<'\n';
        out<<loot.size()<<'\n';for(const auto&item:loot)out<<item.x<<' '<<item.y<<' '<<item.dx<<' '<<item.dy<<' '<<item.age<<' '<<item.cash<<' '<<item.flags<<'\n';        out<<spills.size();for(int tile:spills)out<<' '<<tile;out<<'\n'<<celebratedRank<<'\n';

    }
    void read(std::istream&in){
        Park p=*this;p.rides.clear();p.guests.clear();std::string magic;size_t nr,ng;
        if(!(in>>magic>>p.time>>p.seed>>p.served>>p.spawn>>nr>>ng)||(magic!="TPPLAY2"&&magic!="TPPLAY3"&&magic!="TPPLAY4"&&magic!="TPPLAY5"&&magic!="TPPLAY6"&&magic!="TPPLAY7"&&magic!="TPPLAY8"&&magic!="TPPLAY9"&&magic!="TPPLAY10"&&magic!="TPPLAY11"&&magic!="TPPLAY12"&&magic!="TPPLAY13")||nr>slots.size()||ng>256)throw std::runtime_error("Invalid save header");
        bool hasParade=magic=="TPPLAY13",hasSpills=hasParade||magic=="TPPLAY12",pickups=magic=="TPPLAY11"||hasSpills,facilities=magic=="TPPLAY10"||pickups,current=magic=="TPPLAY9"||facilities;if(current)magic="TPPLAY8";
        p.staff.clear();p.loot.clear();p.officeTheme=-1;p.officeWings=0;
        p.money=125000;p.tickets=10;p.flags=p.streak=0;if((magic=="TPPLAY6"||magic=="TPPLAY7"||magic=="TPPLAY8")&&!(in>>p.money>>p.tickets>>p.flags>>p.streak))throw std::runtime_error("Truncated economy");
        auto ids=[&](std::vector<int>&v,size_t limit){size_t n;if(!(in>>n)||n>limit)throw std::runtime_error("Invalid save list");v.resize(n);for(int&x:v)if(!(in>>x))throw std::runtime_error("Truncated save");};
        p.lockedSlots.clear();if(magic=="TPPLAY5"||magic=="TPPLAY6"||magic=="TPPLAY7"||magic=="TPPLAY8"){std::vector<int> locked;ids(locked,slots.size());for(int slot:locked){if(slot<0||slot>=int(slots.size())||!p.lockedSlots.insert(slot).second)throw std::runtime_error("Invalid locked plot");}}
        for(size_t i=0;i<nr;i++){Ride r;if(!(in>>r.slot>>r.type>>r.rotation>>r.open>>r.served>>r.timer>>r.boarding))throw std::runtime_error("Truncated ride");ids(r.queue,96);ids(r.riders,96);if((magic=="TPPLAY4"||magic=="TPPLAY5"||magic=="TPPLAY6"||magic=="TPPLAY7"||magic=="TPPLAY8")&&!(in>>r.gateLevel))throw std::runtime_error("Truncated gate level");if((magic=="TPPLAY6"||magic=="TPPLAY7"||magic=="TPPLAY8")&&!(in>>r.profit>>r.rewardFlags>>r.incomeClock>>r.challenge>>r.happiness>>r.goodTime>>r.tapSpeed>>r.animationTime))throw std::runtime_error("Truncated ride economy");p.rides.push_back(r);}
        for(size_t i=0;i<ng;i++){Guest g;int state;if(!(in>>g.tile>>g.target>>g.visits>>state>>g.step>>g.progress>>g.wait>>g.walkTime))throw std::runtime_error("Truncated guest");g.state=State(state);ids(g.route,tiles.size());p.guests.push_back(g);}
        p.openTerritories.clear();if(magic=="TPPLAY7"||magic=="TPPLAY8"){std::vector<int> opened;ids(opened,20);for(int t:opened){if(t<0||t>=20||!p.openTerritories.insert(t).second)throw std::runtime_error("Invalid territory");}}else if(p.enforceTerritories){p.openTerritories.insert(17);for(auto&r:p.rides)p.openTerritories.insert(p.territory(r.slot));}
        p.themes.fill(0);if(magic=="TPPLAY8")for(int&theme:p.themes)if(!(in>>theme)||theme<0||theme>11)throw std::runtime_error("Invalid territory theme");
        // Older saves used six seats for every attraction; use recovered locator
        // capacities and release excess passengers directly at the gate.
        if(magic!="TPPLAY7"&&magic!="TPPLAY8"&&magic!="TPPLAY9")for(auto&r:p.rides)if(r.type>=0&&r.type<int(p.types.size())&&r.slot>=0&&r.slot<int(p.slots.size())){
            while(r.riders.size()>size_t(p.types[r.type].capacity)){int id=r.riders.back();r.riders.pop_back();if(id<0||id>=int(p.guests.size()))throw std::runtime_error("Invalid passenger");auto&g=p.guests[id];g.tile=p.entrance(r.slot);g.state=State::Idle;g.target=-1;g.route.clear();g.step=0;g.progress=0;g.wait=.5;}
        }
        p.themes[17]=0;
        if(current){for(auto&r:p.rides)if(!(in>>r.buildRemaining>>r.running))throw std::runtime_error("Truncated construction");for(auto&g:p.guests)if(!(in>>g.seat))throw std::runtime_error("Truncated seats");size_t count;if(!(in>>count)||count!=p.bins.size())throw std::runtime_error("Invalid bin count");for(auto&bin:p.bins)if(!(in>>bin.remaining))throw std::runtime_error("Truncated bins");}
        else {for(auto&r:p.rides){r.buildRemaining=0;r.running=!r.riders.empty();r.timer=r.running?20:0;r.boarding=0;r.incomeClock=std::fmod(r.incomeClock,20.);if(r.challenge>=0){r.challenge=0;r.timer=20;r.running=true;r.goodTime=0;}for(size_t seat=0;seat<r.riders.size();seat++){auto&g=p.guests.at(r.riders[seat]);g.seat=int(seat);g.wait=r.timer;}}}
        if(current&&!hasParade)for(auto&bin:p.bins){
            if(bin.remaining<0||bin.remaining>180)throw std::runtime_error("Invalid legacy bin timer");
            if(bin.remaining>0)bin.remaining=std::min(600.,3*bin.remaining);
        }
        bool legacy=magic=="TPPLAY2"||magic=="TPPLAY3";
        if(facilities){size_t count;if(!(in>>p.officeTheme>>p.officeWings>>count)||p.officeTheme< -1||p.officeTheme>2||p.officeWings<0||p.officeWings>7||count>slots.size())throw std::runtime_error("Invalid facilities");for(size_t i=0;i<count;i++){Staff s;if(!(in>>s.slot)||s.slot<0||s.slot>=int(slots.size())||slots[s.slot].w!=4||slots[s.slot].h!=4||p.staffAt(s.slot)>=0||p.occupant(s.slot)>=0||!p.territoryOpen(s.slot)||p.lockedSlots.count(s.slot))throw std::runtime_error("Invalid staff house");if(pickups&&(!(in>>s.kind)||s.kind<0||s.kind>4))throw std::runtime_error("Invalid mascot");s.tile=p.entrance(s.slot);if(s.tile<0||p.tileTerritory(s.tile)!=p.territory(s.slot))throw std::runtime_error("Disconnected staff house");p.staff.push_back(s);}}
        if(pickups){size_t count;if(!(in>>count)||count>10000)throw std::runtime_error("Invalid loot count");for(size_t i=0;i<count;i++){Loot item;if(!(in>>item.x>>item.y>>item.dx>>item.dy>>item.age>>item.cash>>item.flags)||!std::isfinite(item.x)||!std::isfinite(item.y)||!std::isfinite(item.dx)||!std::isfinite(item.dy)||!std::isfinite(item.age)||item.age<0||item.age>10||item.cash<0||item.flags<0||item.cash>2000000||item.flags>10000)throw std::runtime_error("Invalid loot");p.loot.push_back(item);}}
        p.spills.clear();p.pickedItems.clear();if(hasSpills){size_t count;if(!(in>>count)||count>12)throw std::runtime_error("Invalid spills");for(size_t i=0;i<count;i++){int tile;if(!(in>>tile)||tile<0||tile>=int(tiles.size()))throw std::runtime_error("Invalid spill tile");p.spills.insert(tile);}}
        p.celebratedRank=p.rank();if(hasParade&&(!(in>>p.celebratedRank)||p.celebratedRank<1||p.celebratedRank>p.rank()))throw std::runtime_error("Invalid parade rank");
        if(in>>magic)throw std::runtime_error("Trailing save data");p.validate();
        if(legacy){for(auto&r:p.rides){for(int id:r.riders)p.guests[id].wait=r.timer;for(int id:r.queue){auto&g=p.guests[id];g.state=State::Idle;g.wait=.5;g.target=-1;}r.queue.clear();r.boarding=0;}p.validate();}*this=std::move(p);
    }
};
}
