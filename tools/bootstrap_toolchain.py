"""Fetch pinned official Zig C/C++ toolchain into .tools; no global install."""
import hashlib,json,pathlib,shutil,urllib.request,zipfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
VERSION='0.13.0'
def main():
    folder=ROOT/'.tools';folder.mkdir(exist_ok=True)
    metadata=json.load(urllib.request.urlopen('https://ziglang.org/download/index.json',timeout=30))[VERSION]['x86_64-windows']
    url=metadata['tarball']
    if not url.startswith('https://ziglang.org/download/'+VERSION+'/'):raise ValueError('Unexpected toolchain origin')
    target=folder/pathlib.PurePosixPath(url).name
    def digest(p):
        with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
    if not target.exists() or digest(target)!=metadata['shasum']:
        print('Downloading official Zig',VERSION,metadata['size'],'bytes',flush=True)
        with urllib.request.urlopen(url,timeout=60) as response,target.open('wb') as dest:shutil.copyfileobj(response,dest)
    if digest(target)!=metadata['shasum']:raise ValueError('Toolchain SHA256 mismatch')
    with zipfile.ZipFile(target) as z:
        for member in z.infolist():
            path=(folder/member.filename).resolve()
            if not path.is_relative_to(folder.resolve()):raise ValueError('Unsafe toolchain ZIP member')
        z.extractall(folder)
    (folder/'toolchain.json').write_text(json.dumps({'version':VERSION,**metadata},indent=2))
    print('Verified toolchain ready:',folder/f'zig-windows-x86_64-{VERSION}'/'zig.exe',flush=True)
if __name__=='__main__':main()
