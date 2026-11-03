"""Stage original theme assets; never modify source packages.
Path choice: ThemeSlot::getPathTexture, VA 0xc067c.
Fence table: RideSlot::_Construct MODELS_SLOTS_THEMED_*_values.
Ground choices: ThemeSlot::createDecorations, VA 0xc2dac.
Gate families: GameMap::_Construct named Gate_values arrays.
"""
import json, struct, zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    symbols=json.loads((ROOT/'analysis/ios/symbols.json').read_text())
    with zipfile.ZipFile(ROOT/'resource/Theme_Park_1.8.3_ios_5.1.1.ipa') as archive:
        binary=archive.read('Payload/ThemePark.app/ThemePark')
    fences={}
    for s in symbols:
        if not s['is_debug'] and 'MODELS_SLOTS_THEMED_' in s['name']:
            index=int(s['name'].split('THEMED_')[1].split('_')[0])
            fences[index]=struct.unpack_from('<4i',binary,s['file_offset'])
    families=["funland","funland","horror","future","sky","caribbean","cowboy","snow","atlantis","candy","space","waterworld"]
    walls={}
    for symbol in symbols:
        if not symbol["is_debug"] and "GameMap10_Construct" in symbol["name"] and "Walls_values" in symbol["name"]:
            for family in families:
                if family+"Walls_values" in symbol["name"]: walls[family]=struct.unpack_from("<3i",binary,symbol["file_offset"])
    names=['Classic','Carnival','Haunted','Alien','Sky','Pirate','Western','Snow','Underwater','Candy','Space','Waterworld']
    statues=[0,335,340,345,365,355,350,360,370,375,380,383]
    paths=[306,297,298,300,301,296,299,306,302,303,304,305]
    grounds=[392,392,393,395,397,396,394,398,399,401,400,402]
    gates=[216,216,220,224,228,208,212,232,244,240,236,248]
    models=json.loads((ROOT/'analysis/models.json').read_text())['models']
    lines=[]
    for i,name in enumerate(names):
        base=fences[i]
        # Use the exact original theme fence table, without guessing adjacent variants.
        built=walls[families[i]]
        tex=models[paths[i]]['parts'][0]['texture']
        lines.append(f'{i} {statues[i]} {grounds[i]} {tex} {built[0]} {built[1]} {built[2]} {gates[i]} {name}')
    (ROOT/'extracted/runtime/territory_themes.tsv').write_text('\n'.join(lines)+'\n')
    data=json.loads((ROOT/'extracted/level_data/Theme_Park_1.8.3_ios_5.1.1/named/themeSlots.json').read_text())
    rows=[]
    for _,x,y,w,h,*rest in data['rows']:
        territory=int((y+h/2-1)//36)*5+int((x+w/2-1)//36)
        rows.append((territory,x,y,w,h))
    assert len(set(r[0] for r in rows))==20
    (ROOT/'extracted/runtime/theme_plots.tsv').write_text(''.join(' '.join(map(str,r))+'\n' for r in rows))
    dec=json.loads((ROOT/'extracted/level_data/Theme_Park_1.8.3_ios_5.1.1/named/themedDecData.json').read_text())['rows']
    bins=[(x,y,int((y-1)//36)*5+int((x-1)//36)) for kind,x,y,w,h,angle,owner in dec if kind==4]
    (ROOT/'extracted/runtime/trash_bins.tsv').write_text(''.join(' '.join(map(str,r))+'\n' for r in bins))
    print('12 theme choices; 20 reserved plots; path/fence provenance preserved')
if __name__=='__main__': main()
