"""Read a local reference video and sample frames; never modify its source."""
import hashlib,json,re,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools/_deps'))
import imageio_ffmpeg

def sha(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream,'sha256').hexdigest()

def main():
    videos=list((ROOT/'resource').glob('*.mp4'))
    if len(videos)!=1:
        raise ValueError('Expected one local MP4 reference in resource/')
    source=videos[0];before=sha(source);ffmpeg=imageio_ffmpeg.get_ffmpeg_exe()
    info=subprocess.run([ffmpeg,'-hide_banner','-i',str(source)],capture_output=True,text=True).stderr
    duration=re.search(r'Duration: (\d+):(\d+):(\d+\.\d+)',info)
    if not duration or 'Video:' not in info:
        raise ValueError('No readable video stream')
    hh,mm,ss=map(float,duration.groups());seconds=hh*3600+mm*60+ss
    out=ROOT/'analysis/previews/gameplay';out.mkdir(parents=True,exist_ok=True)
    frames=[]
    for timestamp in range(30,int(seconds),60):
        path=out/f'frame-{timestamp:04d}s.png'
        subprocess.run([ffmpeg,'-hide_banner','-loglevel','error','-y','-ss',str(timestamp),
                        '-i',str(source),'-frames:v','1',str(path)],check=True)
        frames.append(dict(requested_seconds=timestamp,path=path.relative_to(ROOT).as_posix()))
    after=sha(source)
    if before!=after:raise ValueError('Reference changed during inspection')
    report=dict(source=source.relative_to(ROOT).as_posix(),sha256=before,sha256_after=after,
                duration_seconds=seconds,ffmpeg_metadata=info,frames=frames,
                scope='Sparse visual reference, not a full gameplay or version validation')
    (out/'reference.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    # Optional contact sheet preserves the 16:9 source aspect ratio.
    try:
        from PIL import Image,ImageDraw
    except ImportError:
        pass
    else:
        sheet=Image.new('RGB',(960,206*((len(frames)+2)//3)),'#18212a');draw=ImageDraw.Draw(sheet)
        for i,frame in enumerate(frames):
            x,y=(i%3)*320,(i//3)*206
            with Image.open(ROOT/frame['path']) as im:sheet.paste(im.resize((320,180)),(x,y))
            t=frame['requested_seconds'];draw.text((x+6,y+184),f'{t//60:02d}:{t%60:02d}',fill='white')
        sheet.save(out/'contact-sheet.jpg')
    print(json.dumps(dict(duration_seconds=seconds,sampled_frames=len(frames),source_unchanged=before==after)))

if __name__=='__main__':main()
