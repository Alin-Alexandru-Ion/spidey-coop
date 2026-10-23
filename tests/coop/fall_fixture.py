"""Read installed rooftop trigger metadata into a temporary native test fixture."""
import struct,zlib
from pathlib import Path
def generate_one(repo,work,bank,prefix):
    data=(repo.parent / "Spider-Man-decomp-baseline/data.pkr").read_bytes()
    offset=struct.unpack_from("<I",data,4)[0]
    _,dirs,files=struct.unpack_from("<III",data,offset)
    offset+=12+dirs*40
    for i in range(files):
        name,crc,compressed,pos,size,packed=struct.unpack_from("<32sIiIII",data,offset+i*52)
        if name.split(b"\0")[0].lower()==bank:
            raw=data[pos:pos+packed];raw=zlib.decompress(raw) if compressed==2 else raw;break
    else:raise AssertionError("Rooftop trigger asset missing")
    count=struct.unpack_from("<I",raw,8)[0]
    offsets=list(struct.unpack_from("<"+"I"*count,raw,12))+[len(raw)]
    lines=[];setup=[]
    for i in range(count):
        start,end=offsets[i:i+2];words=struct.unpack_from("<"+"H"*((end-start)//2),raw,start)
        lines.append("static u16 %snode%d[]={%s};"%(prefix,i,",".join(map(str,words))))
        setup.append("nodes[%d]=%snode%d;"%(i,prefix,i))
        if words[0]==6:
            # Native parser aligns past links before reading the checksum.
            checksum_pos=(start+4+words[1]*2+3)&~3
            command=(checksum_pos+4-start)//2
            checksum=struct.unpack_from("<I",raw,checksum_pos)[0]
            setup.append("points[%d].NodeIndex=%d;points[%d].pCommands=%snode%d+%d;points[%d].Checksum=%du;"%(i,i,i,prefix,i,command,i,checksum))
            setup.append("points[%d].pNext=pointList;pointList=&points[%d];"%(i,i))
            setup.append("points[%d].pNextSimilar=hashTable[%d&255];hashTable[%d&255]=&points[%d];"%(i,checksum,checksum,i))
    lines.append("static void Load%sFallFixture(){memset(points,0,sizeof(points));memset(hashTable,0,sizeof(hashTable));pointList=0;nodeCount=%d;"%(prefix,count)+"".join(setup)+"}")
    return "\n".join(lines)
def generate(repo,work):
    (work / "fall_fixture.h").write_text(generate_one(repo,work,b"l1a1_t.trg","")+"\n"+generate_one(repo,work,b"l1a4_t.trg","Bomb"))
