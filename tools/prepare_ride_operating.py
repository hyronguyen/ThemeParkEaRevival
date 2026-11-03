"""Inferred assembly/group pairing from the ordered original model/animation catalogs.
Not a recovered server mapping. Validate hierarchy, moving nodes and native captures.
"""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BINDINGS={403:32,409:34,410:35,416:38,418:39,419:40,420:41,421:42,422:43,423:44,424:45,425:46,427:48,428:50,429:51,430:52,431:55,433:60,434:61,436:63,438:66,440:69,441:70,443:71,444:72,445:73,447:74,449:75,450:78,452:81,453:82,454:83,455:86,456:87,458:88,460:89,461:90,463:91,465:94,467:97,469:98,470:99,471:100,472:101,473:102,474:103,475:104,476:107,477:108,479:113,484:118,486:119,487:120,488:121,489:122,490:123,491:124,492:125,493:128,495:129,497:132,498:133,499:134,500:135,502:136,503:137,508:138,509:139,511:140,512:143,513:146,514:147,515:150,517:152,519:155,520:156,521:157,522:158,529:160,531:161,533:162,534:163,535:166,536:167,537:170,538:171,539:172,540:173,541:176,542:177,543:180,544:181}
def main():
    a=json.loads((ROOT/'analysis/animations.json').read_text())
    models=json.loads((ROOT/'analysis/models.json').read_text())['models']
    rows=[]
    for model,group in BINDINGS.items():
        g=a['groups'][group]
        assert len(models[model]['nodes'])==g['nodes'], (model,group)
        if any(abs(v)>=1e8 for tid in g['tracks'] for c in a['tracks'][tid]['channels'] for v in c):
            print('Unsafe transform range; binding withheld:',model,group);continue
        # No terrain/root displacement; non-identity static root offsets are rejected.
        t=a['tracks'][g['tracks'][0]]
        identity=[1,1,1,0,0,0,1,0,0,0]
        assert all(abs(v-identity[i])<.001 for i,c in enumerate(t['channels']) for v in c), (model,group,'root')
        moving=[n for n,tid in enumerate(g['tracks']) if any(max(c)-min(c)>.001 for c in a['tracks'][tid]['channels'])]
        if not moving:
            print('Static assembly skipped:',model,group);continue
        rows.append(dict(model=model,group=group,moving_nodes=moving,status='inferred_ordered_catalog_binding'))
    (ROOT/'extracted/runtime/operating_animations.tsv').write_text('# Original clips; inferred ordered-catalog bindings, not server-confirmed.\n'+''.join(f'{r["model"]}\t{r["group"]}\tanimations/ios/group_{r["group"]:04d}.tpa\n' for r in rows))
    (ROOT/'analysis/operating_catalog_audit.json').write_text(json.dumps(rows,indent=2))
    print('Validated',len(rows),'stationary-root operating candidates')
if __name__=='__main__': main()
