"""Read-only APK evidence collector, using only the Python standard library.

Parses Android binary XML, DEX index tables and ELF32/64 section metadata.
Does not execute original code or connect to any service.
"""
from __future__ import annotations
import argparse, collections, hashlib, json, re, struct, zipfile, zlib
from pathlib import Path

def u16(b, o): return struct.unpack_from('<H', b, o)[0]
def u32(b, o): return struct.unpack_from('<I', b, o)[0]
def cstr(b, o): return b[o:b.find(b'\0', o)].decode('utf-8', 'replace')
def uleb(b, o):
    value = shift = 0
    while True:
        v=b[o]; o+=1; value|=(v&127)<<shift
        if not v&128: return value,o
        shift+=7

def axml(b):
    if b[:2]!=b'\x03\x00': return {'error':'not Android binary XML'}
    pool=[]; nodes=[]; depth=0; o=u16(b,2)
    while o+8<=len(b):
        typ,head,size=struct.unpack_from('<HHI',b,o)
        if size<8 or o+size>len(b): raise ValueError('invalid AXML chunk')
        if typ==1:
            count=u32(b,o+8); flags=u32(b,o+16); start=u32(b,o+20)
            for i in range(count):
                p=o+start+u32(b,o+head+i*4)
                if flags&256:
                    for _ in range(2):
                        n=b[p];p+=1
                        if n&128: n=((n&127)<<8)|b[p];p+=1
                    pool.append(b[p:p+n].decode('utf-8','replace'))
                else:
                    n=u16(b,p);p+=2
                    if n&32768: n=((n&32767)<<16)|u16(b,p);p+=2
                    pool.append(b[p:p+n*2].decode('utf-16le','replace'))
        elif typ==0x102:
            p=o+head; name=pool[u32(b,p+4)]; attrstart=u16(b,p+8); attrsize=u16(b,p+10); count=u16(b,p+12)
            attrs={}
            for i in range(count):
                a=p+attrstart+i*attrsize; aname=pool[u32(b,a+4)]; raw=u32(b,a+8); kind=b[a+15]; value=u32(b,a+16)
                if raw!=0xffffffff: val=pool[raw]
                elif kind==3: val=pool[value]
                elif kind==0x12: val=bool(value)
                elif kind in (0x10,0x11): val=value
                else: val=f'0x{value:08x} (type 0x{kind:02x})'
                attrs[aname]=val
            nodes.append({'depth':depth,'tag':name,'attributes':attrs,'offset':o}); depth+=1
        elif typ==0x103: depth-=1
        o+=size
    return {'nodes':nodes}

def dex(b):
    if not b.startswith(b'dex\n'): raise ValueError('not DEX')
    strings=[]
    for i in range(u32(b,56)):
        off=u32(b,u32(b,60)+i*4);_,p=uleb(b,off)
        strings.append(cstr(b,p))
    types=[strings[u32(b,u32(b,68)+i*4)] for i in range(u32(b,64))]
    classes=[]
    for i in range(u32(b,96)):
        o=u32(b,100)+i*32;idx,access,superidx,interfaces,source,annotations,clsdata,static=struct.unpack_from('<8I',b,o)
        classes.append({'name':types[idx],'super':types[superidx] if superidx<len(types) else None,'source':strings[source] if source<len(strings) else None,'class_data_offset':clsdata})
    protos=[]
    for i in range(u32(b,72)):
        o=u32(b,76)+i*12;shorty,ret,pars=struct.unpack_from('<III',b,o)
        params=[types[u16(b,pars+4+j*2)] for j in range(u32(b,pars))] if pars else []
        protos.append('('+''.join(params)+')'+types[ret])
    methods=[]
    for i in range(u32(b,88)):
        o=u32(b,92)+i*8;cls,proto,name=struct.unpack_from('<HHI',b,o)
        methods.append({'index':i,'class':types[cls],'name':strings[name],'signature':protos[proto]})
    defined=[]
    for cls in classes:
        p=cls['class_data_offset']
        if not p: continue
        counts=[]
        for _ in range(4): value,p=uleb(b,p); counts.append(value)
        for _ in range(counts[0]+counts[1]): _,p=uleb(b,p);_,p=uleb(b,p)
        for count in counts[2:]:
            idx=0
            for _ in range(count):
                diff,p=uleb(b,p);access,p=uleb(b,p);code,p=uleb(b,p);idx+=diff
                defined.append({**methods[idx],'access_flags':access,'native':bool(access&256),'code_offset':code})
    # Walk actual instruction boundaries; never interpret arbitrary aligned bytes
    # as references. Payload pseudo-instructions have variable widths.
    widths=[1]*256
    for opcode in [2,5,8,0x13,0x15,0x16,0x19,0x1a,0x1c,0x1f,0x20,0x22,0x23,0x29]+list(range(0x2d,0x3e))+list(range(0x44,0x6e))+list(range(0x90,0xb0))+list(range(0xd0,0xe3)): widths[opcode]=2
    for opcode in [3,6,9,0x14,0x17,0x1b,0x24,0x25,0x26,0x2a,0x2b,0x2c]+list(range(0x6e,0x73))+list(range(0x74,0x79)): widths[opcode]=3
    widths[0x18]=5
    for m in defined:
        code=m['code_offset']
        if not code: continue
        count=u32(b,code+12); units=list(struct.unpack_from('<'+str(count)+'H',b,code+16));pc=0;refs=[]
        while pc<count:
            unit=units[pc];op=unit&255;w=widths[op]
            if op==0 and unit>>8:
                kind=unit>>8
                if kind==1:w=4+units[pc+1]*2
                elif kind==2:w=2+units[pc+1]*4
                elif kind==3:w=4+(units[pc+1]*(units[pc+2]|(units[pc+3]<<16))+1)//2
                else:raise ValueError('unknown DEX payload')
            if pc+w>count:raise ValueError('DEX instruction beyond method')
            ref={'code_unit_offset':pc,'file_offset':code+16+pc*2,'opcode':f'0x{op:02x}'}
            if op in (0x1a,0x1b):
                idx=units[pc+1]|((units[pc+2]<<16) if op==0x1b else 0);ref.update(kind='string',value=strings[idx]);refs.append(ref)
            elif 0x6e<=op<=0x72 or 0x74<=op<=0x78:
                ref.update(kind='invoke',target=methods[units[pc+1]]);refs.append(ref)
            elif op in (0x13,0x14,0x15,0x16,0x17,0x18,0x19):
                raw=0
                for k in range(1,w):raw|=units[pc+k]<<(16*(k-1))
                ref.update(kind='constant_raw',value=raw,register=unit>>8);refs.append(ref)
            pc+=w
        m['instruction_units']=count;m['references']=refs
    return {'magic':b[:8].decode('ascii','replace'),'checksum_valid':u32(b,8)==zlib.adler32(b[12:]),'signature_valid':b[12:32]==hashlib.sha1(b[32:]).digest(),'header_size_matches':u32(b,32)==len(b),'strings_count':len(strings),'types_count':len(types),'classes_count':len(classes),'method_ids_count':len(methods),'classes':classes,'defined_methods':defined,'strings':strings}

def elf(b):
    if b[:4]!=b'\x7fELF': raise ValueError('not ELF')
    bits=32 if b[4]==1 else 64;endian='<' if b[5]==1 else '>'
    if bits==32: vals=struct.unpack_from(endian+'HHIIIIIHHHHHH',b,16)
    else: vals=struct.unpack_from(endian+'HHIQQQIHHHHHH',b,16)
    etype,machine,version,entry,phoff,shoff,flags,ehsize,phsize,phnum,shsize,shnum,shstridx=vals
    sections=[]
    for i in range(shnum):
        fmt=endian+('IIIIIIIIII' if bits==32 else 'IIQQQQIIQQ')
        name,typ,flags,addr,off,size,link,info,align,entsize=struct.unpack_from(fmt,b,shoff+i*shsize)
        sections.append({'name_idx':name,'type':typ,'flags':flags,'address':addr,'offset':off,'size':size,'link':link,'entsize':entsize})
    sn=sections[shstridx];names=b[sn['offset']:sn['offset']+sn['size']]
    symbols=[];needed=[]
    for s in sections:
        s['name']=cstr(names,s.pop('name_idx'))
        if s['type'] in (2,11):
            st=sections[s['link']];strings=b[st['offset']:st['offset']+st['size']]
            for o in range(s['offset'],s['offset']+s['size'],s['entsize'] or (16 if bits==32 else 24)):
                if bits==32: name,value,size,info,other,index=struct.unpack_from(endian+'IIIBBH',b,o)
                else: name,info,other,index,value,size=struct.unpack_from(endian+'IBBHQQ',b,o)
                if name: symbols.append({'name':cstr(strings,name),'value':value,'size':size,'section_index':index,'type':info&15,'binding':info>>4,'undefined':index==0,'table':s['name']})
        if s['type']==6:
            st=sections[s['link']];strings=b[st['offset']:st['offset']+st['size']]
            for o in range(s['offset'],s['offset']+s['size'],s['entsize'] or (8 if bits==32 else 16)):
                tag,value=struct.unpack_from(endian+('II' if bits==32 else 'QQ'),b,o)
                if tag==1: needed.append(cstr(strings,value))
    return {'bits':bits,'machine':machine,'type':etype,'entry':entry,'sections':sections,'needed':needed,'symbols':symbols}

def printable(b):
    return [{'offset':m.start(),'text':m.group().decode('ascii')} for m in re.finditer(rb'[\x20-\x7e]{6,}',b)]

def inspect(path, out):
    dest=out/path.stem;dest.mkdir(parents=True,exist_ok=True)
    data=path.read_bytes();result={'path':path.as_posix(),'size':len(data),'sha256':hashlib.sha256(data).hexdigest()}
    with zipfile.ZipFile(path) as z:
        result['zip_crc_failure']=z.testzip()
        result['entries']=[{'path':x.filename,'size':x.file_size,'compressed_size':x.compress_size,'compression':x.compress_type,'crc32':f'{x.CRC:08x}'} for x in z.infolist()]
        result['manifest']=axml(z.read('AndroidManifest.xml'))
        result['dex']=[];result['native']=[];result['text_assets']={}
        for name in z.namelist():
            if name.endswith('.dex'):
                parsed=dex(z.read(name)); parsed['path']=name
                (dest/(Path(name).name+'.json')).write_text(json.dumps(parsed,indent=2,ensure_ascii=False),encoding='utf-8')
                result['dex'].append({k:v for k,v in parsed.items() if k not in ('strings','classes','defined_methods')})
                related=[]
                for m in parsed['defined_methods']:
                    if 'progressivemedia' not in m['class'] and 'PMAADActivity' not in m['class']:continue
                    hits=[r for r in m.get('references',[]) if r['kind']=='string' and re.search(r'coin|money|cash|ride|build|guest|save|profile|park|pathfind|visitor|econom|zip|obb|compress|datafile',r['value'],re.I)]
                    if hits:related.append({'class':m['class'],'name':m['name'],'signature':m['signature'],'code_offset':m['code_offset'],'strings':hits})
                (dest/'gameplay_string_xrefs.json').write_text(json.dumps(related,indent=2),encoding='utf-8')
            elif name.endswith('.so'):
                b=z.read(name);parsed=elf(b);parsed['path']=name;parsed['sha256']=hashlib.sha256(b).hexdigest();parsed['strings']=printable(b)
                (dest/(Path(name).name+'.json')).write_text(json.dumps(parsed,indent=2),encoding='utf-8')
                result['native'].append({k:v for k,v in parsed.items() if k not in ('symbols','strings')})
            elif name.endswith(('.cfg','.properties','.xml')) and name.startswith('assets/'):
                result['text_assets'][name]=z.read(name).decode('utf-8','replace')
        (dest/'manifest.json').write_text(json.dumps(result['manifest'],indent=2,ensure_ascii=False),encoding='utf-8')
    return result

def main():
    ap=argparse.ArgumentParser();ap.add_argument('packages',nargs='*',default=['resource/05447-Theme-Park-4.2.1.apk','resource/theme-park.apk']);ap.add_argument('--output',default='analysis/android');args=ap.parse_args()
    out=Path(args.output);out.mkdir(parents=True,exist_ok=True)
    results=[inspect(Path(p),out) for p in args.packages]
    Path('analysis/apk_evidence.json').write_text(json.dumps(results,indent=2,ensure_ascii=False),encoding='utf-8')
    for r in results:
        print(r['path'],r['sha256'])
        print(json.dumps([x for x in r['manifest']['nodes'] if x['tag'] in ('manifest','uses-sdk','uses-feature','uses-permission','application')],indent=2))
        for n in r['native']: print(n['path'],n['needed'])

if __name__=='__main__': main()
