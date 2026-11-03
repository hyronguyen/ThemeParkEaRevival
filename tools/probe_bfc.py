"""Small read-only BFC investigation; previews are derivatives, never replacements."""
import zipfile, struct, zlib, pathlib, json
from PIL import Image, ImageDraw
ROOT = pathlib.Path(__file__).resolve().parents[1]
out = ROOT / 'analysis/previews'
out.mkdir(parents=True, exist_ok=True)
z = zipfile.ZipFile(ROOT / 'resource/Theme_Park_1.8.3_ios_5.1.1.ipa')
head = z.read('Payload/ThemePark.app/head_ipad3.bfc')
cache = {}
items = []
for i in range(min(160, (len(head)-2)//13)):
    r = head[2+i*13:15+i*13]
    key, flag = struct.unpack_from('>IB', r)
    off, pack, size = int.from_bytes(r[5:8], 'big'), r[8], int.from_bytes(r[9:13], 'big')
    if pack not in cache: cache[pack] = z.read(f'Payload/ThemePark.app/{pack}.bfc')
    data = cache[pack][off:off+size]
    if flag != 255: continue
    w,h = struct.unpack_from('<HH',data)
    try:
        raw=zlib.decompress(data[4:])
        if len(raw)!=w*h*4: continue
        im=Image.frombytes('RGBA',(w,h),raw)
        im.save(out/f'bfc_{key:08x}.png')
        items.append((key,w,h,im))
    except (zlib.error,ValueError): pass
for page in range((len(items)+39)//40):
    sheet=Image.new('RGB',(1000,800),(55,61,68)); draw=ImageDraw.Draw(sheet)
    for slot,(key,w,h,im) in enumerate(items[page*40:(page+1)*40]):
        x=(slot%8)*125; y=(slot//8)*160
        im.thumbnail((120,135))
        sheet.paste(im,(x,y),im)
        draw.text((x+3,y+140),f'{key:08x} {w}x{h}',fill='white')
    sheet.save(out/f'probe_{page}.jpg')
print(json.dumps({'decoded':len(items),'previews':str(out)}))
