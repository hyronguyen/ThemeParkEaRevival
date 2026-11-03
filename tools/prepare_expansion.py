"""Stage original UI catalog and evidence-backed decoration rows for native playtest.

Attraction names/categories below are visual catalog labels, not server definitions.
Original files are never modified. Thumbnail IDs refer to the original iPad3 art.
"""
import json
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]

def main():
    rows={int(a,16):b for a,b in (line.split() for line in (ROOT/'extracted/runtime/catalog.tsv').read_text().splitlines() if not line.startswith('#'))}
    (ROOT/'extracted/runtime/ui_assets.tsv').write_text(''.join(f'{i} {p}\n' for i,p in rows.items()))
    with (ROOT/'extracted/runtime/ui_bounds.tsv').open('w') as out:
        for i,p in rows.items():
            with Image.open(ROOT/'extracted'/p) as image:
                im=image.convert('RGBA');b=im.getchannel('A').getbbox() or (0,0,im.width,im.height)
                out.write(f'{i} {b[0]/im.width} {b[1]/im.height} {b[2]/im.width} {b[3]/im.height}\n')
    # Original model catalog, omitting separate rider poses, vehicles and construction pieces.
    rides=[403,404,409,410,414,415,416,418,419,420,421,422,423,424,425,427,428,429,430,431,432,433,434,436,438,440,441,443,444,445,447,449,450,452,453,454,455,456,458,460,461,463,465,467,469,470,471,472,473,474,475,476,477,479,484,486,487,488,489,490,491,492,493,495,497,498,499,500,502,503,508,509,511,512,513,514,515,517,519,520,521,522,529,531,533,534,535,536,537,538,539,540,541,542,543,544]
    shops=list(range(545,583))+list(range(592,618))+[618,619,620]
    names={403:'Stone Castle',404:'Bouncy Castle',409:'Pirate Ship',410:'Fountain Ride',425:'Water Slides',427:'Skull River',432:'Hedge Maze',449:'Teacups',463:'Parasol Chairs',465:'Carousel',476:'Ferris Wheel',515:'Wave Rider',522:'Log Flume',545:'Candy Floss',546:'Candy Shop',547:'Hot Dogs',548:'Octopus Shop',549:'Pumpkin Shop',550:'Fries',551:'Balloons',552:'Coffee',553:'Soft Drinks',554:'Gingerbread',555:'Burgers',556:'Christmas Shop',557:'Fish Shop',558:'Ice Cream',559:'Pizza',560:'Pirate Shop',561:'Gift Shop',562:'Treasure Shop',563:'Saloon',564:'Chicken',565:'Tropical Shop',566:'Toys',567:'Dr Pepper Drinks',568:'Dr Pepper Dice',569:'Dr Pepper Lounge',570:'Party Shop',571:'Tea Garden',572:'Lemonade',573:'Popcorn',574:'Kites',575:'Torch',576:'Workshop',577:'Play Frame',578:'Volcano',579:'Flowers',580:'Picnic Garden',581:'Totem',582:'Tiki Shop',618:'Wooden Kiosk',619:'Phone Booth',620:'Toilets'}
    thumbs={403:0x254,404:0x250,409:0x260,410:0x264,414:0x258,415:0x29c,416:0x2cc,418:0x274,419:0x2c8,420:0x1c6,421:0x1ce,422:0x1d6,423:0x174,424:0x25c,425:0x2f0,427:0x300,428:0x74,429:0x308,430:0x304,431:0x30c,432:0x268,433:0x26c,434:0x270,436:0x27c,438:0x278,440:0x284,441:0x280,443:0x288,444:0x28c,445:0x290,447:0x294,449:0x298,450:0x29c,452:0x2c8,453:0x2e0,454:0x2ac,455:0x2b0,456:0x2d4,458:0x2d0,460:0x2b8,461:0x2bc,463:0x2c4,465:0x2e4,467:0x2f4,469:0x2f8,470:0x2fc,471:0x2e8,472:0x2ec,474:0x2b4,476:0x2a0,477:0x2a4,479:0x2a8,515:0x3a4,517:0x3aa,519:0x3b0,520:0x3b8,521:0x3c0,522:0x3c8,545:0x4a6,546:0x4aa,547:0x4ae,548:0x4b2,549:0x4b6,550:0x462,551:0x466,552:0x46a,553:0x472,554:0x482,555:0x476,556:0x47a,557:0x486,558:0x46e,559:0x47e,560:0x48a,561:0x492,562:0x48e,563:0x49e,564:0x49a,565:0x496,566:0x4a2,567:0x4c4,568:0x4ba,569:0x322,570:0x4ca,571:0x4d0,572:0x4d6,573:0x4dc,574:0x4e2,575:0x4ea,576:0x4f2,577:0x4f6,578:0x4fc,579:0x500,580:0x506,581:0x50e,582:0x514}
    # Visually checked against native model captures; unresolved matches use 3D.
    thumbs.update({419:0,424:0x7a2,428:0x25c})
    # Paired against native renders (detail paintings, never cutouts or texture sheets).
    thumbs.update({414:0,415:0,416:0x2dc,419:0x258,423:0,424:0x2c0,455:0x2d8,
        473:0x2cc,474:0x2b0,475:0x2b4,484:0x328,486:0x326,487:0x324,488:0x322,
        489:0x32e,490:0x334,491:0x33c,492:0x342,493:0x348,495:0x34e,497:0x354,
        498:0x35a,499:0x362,500:0x368,502:0x370,503:0x378,508:0x380,509:0x386,
        511:0x38c,512:0x392,513:0x398,514:0x39e,529:0x3ce,531:0x3d6,533:0x3dc,
        534:0x3e2,535:0x3ea,536:0x3f0,537:0x3f6,538:0x3fe,539:0x406,540:0x40e,
        541:0x414,542:0x41a,543:0x420,544:0x428,567:0x4bc,569:0x4c4})
    # Preserve initial indices used by previous saves.
    order=[425,427,515,522,404]+[i for i in rides+shops if i not in [425,427,515,522,404]]
    records=[dict(model=i,category='SHOPS' if i in shops else 'RIDES',name=names.get(i,f'Original {"Shop" if i in shops else "Ride"} {i}'),thumbnail=thumbs.get(i,0)) for i in order]
    (ROOT/'extracted/runtime/attractions.tsv').write_text(''.join(f'{r["model"]}\t{r["category"]}\t{r["thumbnail"]}\t{r["name"]}\n' for r in records))
    (ROOT/'analysis/attraction_catalog.json').write_text(json.dumps(records,indent=2))
    d=json.loads((ROOT/'extracted/level_data/Theme_Park_1.8.3_ios_5.1.1/named/decorations.json').read_text())
    (ROOT/'extracted/runtime/decorations.tsv').write_text(''.join(' '.join(map(str,r))+'\n' for r in d['rows']))
    print(f'{len(records)} original attraction assemblies; {len(d["rows"])} original decoration rows; {len(rows)} UI textures')
if __name__=='__main__':main()
