"""Verify native PE32+ AMD64 target, imports, absence of CLR, and asset integrity."""
import hashlib,json,pathlib,struct,sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=ROOT/'build'/(sys.argv[1] if len(sys.argv)>1 else 'ThemeParkPreview.exe');data=p.read_bytes()
pe=struct.unpack_from('<I',data,0x3c)[0]
assert data[:2]==b'MZ' and data[pe:pe+4]==b'PE\0\0'
machine,nsects=struct.unpack_from('<HH',data,pe+4)
optional_size=struct.unpack_from('<H',data,pe+20)[0];optional=pe+24
magic=struct.unpack_from('<H',data,optional)[0]
assert machine==0x8664 and magic==0x20b,'Not Windows AMD64 PE32+'
subsystem=struct.unpack_from('<H',data,optional+68)[0]
assert subsystem==2,'Expected native Windows GUI subsystem'
sections=[]
for i in range(nsects):
    s=optional+optional_size+i*40
    virtual_size,rva,raw_size,raw_offset=struct.unpack_from('<IIII',data,s+8)
    sections.append((rva,max(virtual_size,raw_size),raw_offset))
def offset(rva):
    for start,size,off in sections:
        if start<=rva<start+size:return off+rva-start
    raise ValueError('RVA outside sections')
imports_rva=struct.unpack_from('<I',data,optional+112+8)[0]
clr=struct.unpack_from('<II',data,optional+112+14*8)
assert clr==(0,0),'Managed CLR entry present'
imports=[]
if imports_rva:
    pos=offset(imports_rva)
    while any(data[pos:pos+20]):
        name=offset(struct.unpack_from('<I',data,pos+12)[0]);end=data.index(0,name)
        imports.append(data[name:end].decode('ascii'));pos+=20
expected={'kernel32.dll','ntdll.dll','user32.dll','gdi32.dll','gdiplus.dll','ole32.dll','advapi32.dll','shell32.dll','msvcrt.dll','opengl32.dll'}
# The game/workbench binary plays original MP3s through Windows MCI.
if p.stem in {'ThemeParkGame','ThemeParkGame-update','ThemePark3D'}:
    expected.add('winmm.dll')
assert all(name.lower() in expected or name.lower().startswith('api-ms-win-crt-') for name in imports), f'Unexpected runtime dependency {imports}'
result={'machine':'AMD64','optional_header':'PE32+','clr_present':False,'subsystem':subsystem,'imports':imports,'size':len(data),'sha256':hashlib.sha256(data).hexdigest()}
out=ROOT/'analysis/runtime_test';out.mkdir(exist_ok=True)
(out/('pe_verification.json' if p.stem=='ThemeParkPreview' else p.stem+'_pe.json')).write_text(json.dumps(result,indent=2))
print(json.dumps(result,indent=2))
