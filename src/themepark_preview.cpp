// Native preservation workbench. No mobile executable code or assets embedded.
#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace Gdiplus;
namespace fs=std::filesystem;

struct Asset {std::string id; fs::path path;};
struct Tile {int x,y;};
struct Slot {int id,x,y,w,h;};
struct State {int asset=0,mode=0,selected=-1; double mapZoom=4.0,mapX=0,mapY=0,artZoom=1,artX=0,artY=0;};
static fs::path root,savePath;
static std::vector<Asset> assets;
static std::vector<Tile> tiles;
static std::vector<Slot> slots;
static std::unique_ptr<Bitmap> artwork;
static State state;
static std::wstring status=L"Original data loaded. This preview does not simulate gameplay.";
static HWND windowHandle;
static bool dragging=false,smoke=false;
static POINT lastMouse{};
static int clientW=1280,clientH=800;
static ULONG_PTR gdiplusToken;
static int smokeResult=0;

static std::wstring wide(const std::string& s){
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),(int)s.size(),nullptr,0);
    std::wstring result(n,L' '); if(n)MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),(int)s.size(),result.data(),n);return result;
}
static bool loadCatalog(const fs::path& file){
    std::ifstream in(file);if(!in)return false;
    std::vector<Asset> parsed; std::string line;
    while(std::getline(in,line)){
        if(line.empty()||line[0]=='#')continue;
        if(line.back()=='\r')line.pop_back();
        auto tab=line.find('\t');if(tab==std::string::npos)return false;
        auto id=line.substr(0,tab),path=line.substr(tab+1);
        if(id.size()!=8||id.find_first_not_of("0123456789abcdef")!=std::string::npos)return false;
        fs::path rel=fs::u8path(path);
        if(rel.is_absolute()||rel.has_root_name()||path.find(':')!=std::string::npos)return false;
        for(const auto& part:rel)if(part==L"..")return false;
        auto full=root/L"extracted"/rel;
        if(!fs::is_regular_file(full))return false;
        parsed.push_back({id,full});
        if(parsed.size()>20000)return false;
    }
    if(parsed.empty())return false;assets=std::move(parsed);return true;
}
static bool loadLayout(const fs::path& file){
    std::ifstream in(file);if(!in)return false;
    std::vector<Tile> parsed;std::vector<Slot> parsedSlots;std::string line;
    while(std::getline(in,line)){
        if(line.empty()||line[0]=='#')continue;
        char tag,extra;std::istringstream row(line);if(!(row>>tag))return false;
        if(tag=='P'){int x,y;if(!(row>>x>>y)||(row>>extra)||x<0||x>10000||y<0||y>10000)return false;parsed.push_back({x,y});}
        else if(tag=='S'){Slot s;if(!(row>>s.id>>s.x>>s.y>>s.w>>s.h)||(row>>extra)||s.x<0||s.y<0||s.x>10000||s.y>10000||s.w<=0||s.h<=0||s.w>1000||s.h>1000)return false;parsedSlots.push_back(s);}
        else return false;
        if(parsed.size()+parsedSlots.size()>100000)return false;
    }
    if(parsed.empty())return false;tiles=std::move(parsed);slots=std::move(parsedSlots);return true;
}
static bool loadArt(){
    if(state.asset<0||state.asset>=(int)assets.size())return false;
    auto next=std::make_unique<Bitmap>(assets[state.asset].path.c_str());
    if(next->GetLastStatus()!=Ok||!next->GetWidth()||!next->GetHeight())return false;
    artwork=std::move(next);return true;
}
static bool validState(const State& s){
    auto finite=[](double v){return std::isfinite(v)&&std::abs(v)<1000000;};
    return s.asset>=0&&s.asset<(int)assets.size()&&s.mode>=0&&s.mode<=1&&s.selected>=-1&&s.selected<(int)(tiles.size()+slots.size())
        &&finite(s.mapZoom)&&s.mapZoom>=0.2&&s.mapZoom<=50&&finite(s.artZoom)&&s.artZoom>=0.1&&s.artZoom<=16
        &&finite(s.mapX)&&finite(s.mapY)&&finite(s.artX)&&finite(s.artY);
}
static bool saveState(const fs::path& path,const State& s){
    if(!validState(s))return false;
    std::error_code ec;fs::create_directories(path.parent_path(),ec);if(ec)return false;
    auto temp=path;temp+=L".tmp";
    {
        std::ofstream out(temp,std::ios::binary|std::ios::trunc);if(!out)return false;
        out.precision(17);
        out<<"{\n\"schema_version\":1,\n\"asset\":"<<s.asset<<",\n\"mode\":"<<s.mode<<",\n\"selected_record\":"<<s.selected
           <<",\n\"map_zoom\":"<<s.mapZoom<<",\n\"map_x\":"<<s.mapX<<",\n\"map_y\":"<<s.mapY
           <<",\n\"art_zoom\":"<<s.artZoom<<",\n\"art_x\":"<<s.artX<<",\n\"art_y\":"<<s.artY<<"\n}\n";
        out.flush();if(!out)return false;
    }
    return MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
}
static bool readState(const fs::path& path,State& result){
    std::ifstream in(path,std::ios::binary);if(!in)return false;
    std::string data((std::istreambuf_iterator<char>(in)),{});if(data.size()>4096)return false;
    State s;int version=0,end=0;
    int n=std::sscanf(data.c_str()," { \"schema_version\" : %d , \"asset\" : %d , \"mode\" : %d , \"selected_record\" : %d , \"map_zoom\" : %lf , \"map_x\" : %lf , \"map_y\" : %lf , \"art_zoom\" : %lf , \"art_x\" : %lf , \"art_y\" : %lf } %n",
        &version,&s.asset,&s.mode,&s.selected,&s.mapZoom,&s.mapX,&s.mapY,&s.artZoom,&s.artX,&s.artY,&end);
    if(n!=10||version!=1||end!=(int)data.size()||!validState(s))return false;
    result=s;return true;
}
static void resetCamera(){
    if(state.mode==0){state.mapZoom=std::min((clientW-380)/182.0,(clientH-200)/146.0);state.mapZoom=std::clamp(state.mapZoom,0.2,50.0);state.mapX=state.mapY=0;}
    else{state.artZoom=1;state.artX=state.artY=0;}
}
static void pan(double x,double y){if(state.mode==0){state.mapX+=x;state.mapY+=y;}else{state.artX+=x;state.artY+=y;}}
static PointF tilePoint(const Tile& t){return PointF(float((clientW-330)/2.0+(t.x-90)*state.mapZoom+state.mapX),float(115+(clientH-195)/2.0+(t.y-72.5)*state.mapZoom+state.mapY));}
static void zoom(int delta){if(state.mode==0)state.mapZoom=std::clamp(state.mapZoom*std::pow(1.15,delta/120.0),0.2,50.0);else state.artZoom=std::clamp(state.artZoom*std::pow(1.15,delta/120.0),0.1,16.0);}
static void nextAsset(int direction){int old=state.asset;state.asset=(state.asset+direction+(int)assets.size())%(int)assets.size();if(!loadArt()){state.asset=old;status=L"Could not decode resource; retained the previous image.";}else status=L"Original artwork resource "+wide(assets[state.asset].id);}
static void selectPath(int x,int y){
    for(size_t i=0;i<slots.size();++i){auto& s=slots[i];auto p=tilePoint({s.x,s.y});double w=s.w*state.mapZoom,h=s.h*state.mapZoom;
        if(x>=p.X&&x<=p.X+w&&y>=p.Y&&y<=p.Y+h){state.selected=int(tiles.size()+i);status=L"Original ride slot "+std::to_wstring(s.id)+L" | ("+std::to_wstring(s.x)+L", "+std::to_wstring(s.y)+L") "+std::to_wstring(s.w)+L" x "+std::to_wstring(s.h)+L" tiles | inspection only";return;}}
    int selected=-1;double best=std::max(6.0,state.mapZoom)*std::max(6.0,state.mapZoom);
    for(size_t i=0;i<tiles.size();++i){auto p=tilePoint(tiles[i]);double d=(p.X-x)*(p.X-x)+(p.Y-y)*(p.Y-y);if(d<best){best=d;selected=(int)i;}}
    state.selected=selected;
    if(selected>=0)status=L"Original path row "+std::to_wstring(selected)+L" | tile ("+std::to_wstring(tiles[selected].x)+L", "+std::to_wstring(tiles[selected].y)+L")";
    else status=L"No original path tile selected.";
}
static void text(Graphics& g,const std::wstring& value,float x,float y,float width,float height,float size,Color color, bool bold=false){
    Font font(L"Segoe UI",size,bold?FontStyleBold:FontStyleRegular,UnitPixel);SolidBrush brush(color);g.DrawString(value.c_str(),-1,&font,RectF(x,y,width,height),nullptr,&brush);
}
static void drawImageFit(Graphics& g,RectF box,double scale=1,double px=0,double py=0){
    if(!artwork)return;
    float factor=std::min(box.Width/artwork->GetWidth(),box.Height/artwork->GetHeight())*(float)scale;
    float w=artwork->GetWidth()*factor,h=artwork->GetHeight()*factor;
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.DrawImage(artwork.get(),RectF(box.X+(box.Width-w)/2+(float)px,box.Y+(box.Height-h)/2+(float)py,w,h));
}
static void render(Graphics& g,int width,int height){
    g.Clear(Color(255,22,29,35));g.SetSmoothingMode(SmoothingModeAntiAlias);
    SolidBrush top(Color(255,29,40,49)),panel(Color(255,31,41,49)),field(Color(255,36,54,49));
    g.FillRectangle(&top,0,0,width,100);
    text(g,L"THEME PARK  /  PRESERVATION PREVIEW",24,18,float(width-40),34,23,Color(255,238,241,229),true);
    text(g,L"Native Windows x64  •  Original IPA assets and layout  •  Research prototype — not a playable port",25,58,float(width-40),30,14,Color(255,169,190,193));
    int side=width-310;RectF view(20,115,float(side-40),float(height-190));
    g.FillRectangle(&field,view);
    auto clip=g.Save();g.SetClip(view);
    if(state.mode==0){
        SolidBrush tileBrush(Color(255,205,196,151));
        for(auto& t:tiles){auto p=tilePoint(t);float s=float(std::max(1.0,state.mapZoom-0.25));g.FillRectangle(&tileBrush,p.X-s/2,p.Y-s/2,s,s);}
        Pen outline(Color(160,123,172,163),1);
        for(auto& s:slots){auto p=tilePoint({s.x,s.y});g.DrawRectangle(&outline,p.X,p.Y,float(s.w*state.mapZoom),float(s.h*state.mapZoom));}
        if(state.selected>=0){Pen ring(Color(255,255,167,62),3);
            if(state.selected<(int)tiles.size()){auto p=tilePoint(tiles[state.selected]);g.DrawEllipse(&ring,p.X-7,p.Y-7,14.f,14.f);}
            else{auto& s=slots[state.selected-tiles.size()];auto p=tilePoint({s.x,s.y});g.DrawRectangle(&ring,p.X,p.Y,float(s.w*state.mapZoom),float(s.h*state.mapZoom));}}
    }else drawImageFit(g,RectF(view.X+30,view.Y+30,view.Width-60,view.Height-60),state.artZoom,state.artX,state.artY);
    g.Restore(clip);
    g.FillRectangle(&panel,side,115,290,height-190);
    text(g,state.mode==0?L"ORIGINAL PATH LAYOUT":L"ORIGINAL ARTWORK",float(side+18),135,255,30,17,Color(255,230,235,226),true);
    text(g,state.mode==0?L"5,604 path rows • 247 ride slots\nOriginal coordinates and footprints.\nDiagnostic view; no park simulation.":L"Decoded RGBA texture, including\noriginal transparency. No artwork\nwas redrawn or generated.",float(side+18),175,255,75,14,Color(255,173,193,193));
    drawImageFit(g,RectF(float(side+18),270,254,220));
    text(g,L"Resource 0x"+wide(assets[state.asset].id)+L"\n"+std::to_wstring(state.asset+1)+L" / "+std::to_wstring(assets.size())+L" iPad3 RGBA textures",float(side+18),510,260,55,15,Color(255,235,225,192),true);
    SolidBrush button(Color(255,52,72,80));g.FillRectangle(&button,side+18,580,117,36);g.FillRectangle(&button,side+145,580,127,36);
    text(g,L"<  Previous",float(side+25),588,115,28,14,Color::White);text(g,L"Next  >",float(side+171),588,100,28,14,Color::White);
    text(g,status,24,float(height-64),float(width-40),25,13,Color(255,239,210,147));
    text(g,L"Tab: layout / art   •   Wheel: zoom   •   Right drag / arrows: pan   •   PgUp/PgDn: artwork   •   Home: fit   •   S: save   •   L: load",24,float(height-34),float(width-40),24,13,Color(255,161,183,190));
}
static bool capture(const fs::path& path){
    Bitmap bitmap(clientW,clientH,PixelFormat32bppARGB);Graphics g(&bitmap);render(g,clientW,clientH);
    CLSID png={0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
    return bitmap.Save(path.c_str(),&png,nullptr)==Ok;
}
static void keyAction(WPARAM key){
    switch(key){
        case VK_TAB:state.mode=1-state.mode;resetCamera();break;
        case VK_LEFT:case 'A':pan(-24,0);break;
        case VK_RIGHT:case 'D':pan(24,0);break;
        case VK_UP:case 'W':pan(0,-24);break;
        case VK_DOWN:pan(0,24);break;
        case VK_PRIOR:nextAsset(-1);break;
        case VK_NEXT:nextAsset(1);break;
        case VK_HOME:resetCamera();break;
        case 'S':status=saveState(savePath,state)?L"Local preview state saved.":L"Could not save local preview state.";break;
        case 'L':{State loaded;if(readState(savePath,loaded)){auto previous=state;state=loaded;if(loadArt())status=L"Local preview state loaded.";else{state=previous;loadArt();status=L"Saved artwork unavailable; current state retained.";}}else status=L"Save missing or invalid; current state retained.";break;}
        case VK_ESCAPE:DestroyWindow(windowHandle);break;
    }
}
static LRESULT CALLBACK proc(HWND hwnd,UINT msg,WPARAM w,LPARAM l){
    switch(msg){
        case WM_SIZE:clientW=LOWORD(l);clientH=HIWORD(l);return 0;
        case WM_GETMINMAXINFO:reinterpret_cast<MINMAXINFO*>(l)->ptMinTrackSize={1000,760};return 0;
        case WM_ERASEBKGND:return 1;
        case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);Bitmap buffer(std::max(1,clientW),std::max(1,clientH),PixelFormat32bppARGB);Graphics back(&buffer);render(back,clientW,clientH);Graphics screen(dc);screen.DrawImage(&buffer,0,0);EndPaint(hwnd,&ps);return 0;}
        case WM_KEYDOWN:keyAction(w);InvalidateRect(hwnd,nullptr,FALSE);return 0;
        case WM_MOUSEWHEEL:zoom(GET_WHEEL_DELTA_WPARAM(w));InvalidateRect(hwnd,nullptr,FALSE);return 0;
        case WM_RBUTTONDOWN:dragging=true;lastMouse={GET_X_LPARAM(l),GET_Y_LPARAM(l)};SetCapture(hwnd);return 0;
        case WM_RBUTTONUP:dragging=false;ReleaseCapture();return 0;
        case WM_CAPTURECHANGED:dragging=false;return 0;
        case WM_MOUSEMOVE:if(dragging){POINT now={GET_X_LPARAM(l),GET_Y_LPARAM(l)};pan(now.x-lastMouse.x,now.y-lastMouse.y);lastMouse=now;InvalidateRect(hwnd,nullptr,FALSE);}return 0;
        case WM_LBUTTONDOWN:{int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l),side=clientW-310;if(y>=580&&y<=616&&x>=side+18){if(x<=side+135)nextAsset(-1);else if(x>=side+145&&x<=side+272)nextAsset(1);}else if(state.mode==0&&x<side-20&&y>=115&&y<clientH-75)selectPath(x,y);InvalidateRect(hwnd,nullptr,FALSE);return 0;}
        case WM_TIMER:if(smoke){
            auto output=root/L"analysis"/L"runtime_test";fs::create_directories(output);
            State start=state;SendMessageW(hwnd,WM_KEYDOWN,VK_RIGHT,0);bool moved=state.mapX==start.mapX+24;
            SendMessageW(hwnd,WM_MOUSEWHEEL,MAKEWPARAM(0,WHEEL_DELTA),0);bool zoomed=state.mapZoom>start.mapZoom;
            auto first=tilePoint(tiles[0]);SendMessageW(hwnd,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM((int)first.X,(int)first.Y));bool picked=state.selected>=0;
            auto slot=tilePoint({slots[0].x+slots[0].w/2,slots[0].y+slots[0].h/2});SendMessageW(hwnd,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM((int)slot.X,(int)slot.Y));bool slotPicked=state.selected>=(int)tiles.size();
            double beforeDrag=state.mapX;SendMessageW(hwnd,WM_RBUTTONDOWN,MK_RBUTTON,MAKELPARAM(200,200));SendMessageW(hwnd,WM_MOUSEMOVE,MK_RBUTTON,MAKELPARAM(225,210));SendMessageW(hwnd,WM_RBUTTONUP,0,MAKELPARAM(225,210));bool dragged=state.mapX==beforeDrag+25;
            SendMessageW(hwnd,WM_KEYDOWN,'S',0);State saved=state;SendMessageW(hwnd,WM_KEYDOWN,VK_LEFT,0);SendMessageW(hwnd,WM_KEYDOWN,'L',0);bool restored=state.mapX==saved.mapX;
            bool mapShot=capture(output/L"native-layout.png");
            SendMessageW(hwnd,WM_KEYDOWN,VK_TAB,0);bool toggled=state.mode==1;SendMessageW(hwnd,WM_KEYDOWN,VK_NEXT,0);bool changed=state.asset!=start.asset;
            SendMessageW(hwnd,WM_KEYDOWN,VK_PRIOR,0);bool artShot=capture(output/L"native-artwork.png");
            smokeResult=moved&&zoomed&&picked&&slotPicked&&dragged&&restored&&toggled&&changed&&mapShot&&artShot?0:3;
            std::ofstream report(output/L"smoke_test.json");report<<"{\"launch\":true,\"keyboard_pan\":"<<moved<<",\"wheel_zoom\":"<<zoomed<<",\"mouse_pick\":"<<picked<<",\"slot_pick\":"<<slotPicked<<",\"right_drag\":"<<dragged<<",\"save_load\":"<<restored<<",\"view_toggle\":"<<toggled<<",\"art_navigation\":"<<changed<<",\"screenshots\":"<<(mapShot&&artShot)<<",\"exit_code\":"<<smokeResult<<"}\n";
            DestroyWindow(hwnd);
        }return 0;
        case WM_DESTROY:PostQuitMessage(smokeResult);return 0;
    }return DefWindowProcW(hwnd,msg,w,l);
}
static int selfTest(){
    auto output=root/L"analysis"/L"runtime_test";fs::create_directories(output);
    int decoded=0;for(const auto& asset:assets){Bitmap b(asset.path.c_str());if(b.GetLastStatus()!=Ok||!b.GetWidth()||!b.GetHeight())return 10;++decoded;}
    Bitmap missing((output/L"nonexistent.png").c_str());if(missing.GetLastStatus()==Ok)return 11;
    State a;a.asset=std::min(8,(int)assets.size()-1);a.selected=0;a.mapX=37.25;a.mapY=-18.5;a.artZoom=1.75;State b;
    if(!saveState(output/L"roundtrip.json",a)||!readState(output/L"roundtrip.json",b)||a.mapX!=b.mapX||a.mapY!=b.mapY||a.asset!=b.asset||a.artZoom!=b.artZoom)return 12;
    {std::ofstream bad(output/L"corrupt.json");bad<<"{\"schema_version\":999}";}
    if(readState(output/L"corrupt.json",b))return 13;
    a.mapZoom=std::nan("");if(saveState(output/L"invalid.json",a))return 14;
    std::ofstream report(output/L"self_test.json");report<<"{\"native_x64\":true,\"pngs_decoded\":"<<decoded<<",\"original_path_rows\":"<<tiles.size()<<",\"missing_image_rejected\":true,\"save_roundtrip\":true,\"corrupt_save_rejected\":true,\"nonfinite_state_rejected\":true}\n";
    return 0;
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR command,int show){
    SetProcessDPIAware();GdiplusStartupInput startup;if(GdiplusStartup(&gdiplusToken,&startup,nullptr)!=Ok)return 1;
    int result=1;
    try{
        wchar_t exe[32768];GetModuleFileNameW(nullptr,exe,32768);root=fs::path(exe).parent_path().parent_path();
        std::wstring args=command;smoke=args.find(L"--smoke-test")!=std::wstring::npos;
        bool test=args.find(L"--self-test")!=std::wstring::npos;
        wchar_t local[32768];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",local,32768);
        savePath=(n&&n<32768?fs::path(local):root/L"build")/L"ThemeParkPreservation"/L"preview.json";
        if(smoke)savePath=root/L"analysis"/L"runtime_test"/L"smoke-preview.json";
        if(!loadCatalog(root/L"extracted/runtime/catalog.tsv")||!loadLayout(root/L"extracted/runtime/park_layout.tsv")||!loadArt()){
            if(!test&&!smoke)MessageBoxW(nullptr,L"Original resources are missing or invalid. Run tools/extract_assets.py and tools/export_park_layout.py first. Keep this EXE in the build folder.",L"Theme Park preservation preview",MB_OK|MB_ICONERROR);
            result=2;
        }else if(test)result=selfTest();
        else{
            WNDCLASSW wc{};wc.lpfnWndProc=proc;wc.hInstance=instance;wc.lpszClassName=L"ThemeParkPreservationPreview";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);RegisterClassW(&wc);
            windowHandle=CreateWindowExW(0,wc.lpszClassName,L"Theme Park preservation preview — not a playable port",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1280,840,nullptr,nullptr,instance,nullptr);
            if(!windowHandle)result=3;
            else{resetCamera();ShowWindow(windowHandle,show);UpdateWindow(windowHandle);if(smoke)SetTimer(windowHandle,1,800,nullptr);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}result=(int)msg.wParam;}
        }
    }catch(const std::exception& e){if(!smoke)MessageBoxW(nullptr,wide(e.what()).c_str(),L"Preview error",MB_OK|MB_ICONERROR);result=4;}
    artwork.reset();GdiplusShutdown(gdiplusToken);return result;
}
