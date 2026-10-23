"""Use the installed bank scripts, including the actual five-command scene node."""
import struct,zlib

def generate(repo,work):
    data=(repo.parent/'Spider-Man-decomp-baseline/data.pkr').read_bytes()
    off=struct.unpack_from('<I',data,4)[0]
    _,dirs,files=struct.unpack_from('<III',data,off);off+=12+dirs*40
    for i in range(files):
        name,crc,compressed,pos,size,packed=struct.unpack_from('<32sIiIII',data,off+i*52)
        if name.split(b'\0')[0].lower()==b'l1a3_t.trg':
            raw=data[pos:pos+packed];raw=zlib.decompress(raw) if compressed==2 else raw;break
    else:raise AssertionError('Bank trigger asset missing')
    count=struct.unpack_from('<I',raw,8)[0]
    offsets=list(struct.unpack_from('<'+'I'*count,raw,12))+[len(raw)]
    lines=[]
    for node in (38,39,67,68,70,71):
        start,end=offsets[node:node+2]
        kind,links=struct.unpack_from('<HH',raw,start);assert kind==6
        begin=((start+4+2*links+3)&~3)+4
        words=struct.unpack_from('<'+'H'*((end-begin)//2),raw,begin)
        lines.append('static unsigned short bank%d[]={%s};'%(node,','.join(map(str,words))))
    (work/'bank_fixture.h').write_text('\n'.join(lines))
    # Sample opposing room transitions from later installed banks. These are
    # actual command words, so the same decoder and ownership policy are tested.
    late={
        b'l7a1_t.trg':('Office',44,46),
        b'l7a2_t.trg':('Cables',135,121),
        b'l7a3_t.trg':('Press',134,115),
        b'l7a4_t.trg':('Furnace',102,105),
        b'l8a1_t.trg':('Warehouse',252,251),
        b'l8a2_t.trg':('Gear',52,53),
        b'l8a3_t.trg':('Control',88,86),
    }
    extra=[]
    for bank,(label,hide,show) in late.items():
        for i in range(files):
            name,crc,compressed,pos,size,packed=struct.unpack_from('<32sIiIII',data,off+i*52)
            if name.split(b'\0')[0].lower()==bank:
                raw=data[pos:pos+packed]
                if compressed==2:raw=zlib.decompress(raw)
                break
        else:raise AssertionError('Later bank missing: '+bank.decode())
        count=struct.unpack_from('<I',raw,8)[0]
        offsets=list(struct.unpack_from('<'+'I'*count,raw,12))+[len(raw)]
        for suffix,node in (('Hide',hide),('Show',show)):
            start,end=offsets[node:node+2]
            kind,links=struct.unpack_from('<HH',raw,start)
            assert kind==6 and links<1000
            begin=((start+4+2*links+3)&~3)+4
            words=struct.unpack_from('<'+'H'*((end-begin)//2),raw,begin)
            assert words[0]==0xbf,(bank,node,words[0])
            extra.append('static unsigned short late%s%s[]={%s};'%(label,suffix,','.join(map(str,words))))
    # The helicopter chase's abandoned-building handoff loads another bank.
    # Keep the shared-context dispatch test tied to the installed mission data.
    for i in range(files):
        name,crc,compressed,pos,size,packed=struct.unpack_from('<32sIiIII',data,off+i*52)
        if name.split(b'\0')[0].lower()==b'l3a1_t.trg':
            chase=data[pos:pos+packed]
            chase=zlib.decompress(chase) if compressed==2 else chase
            break
    else:raise AssertionError('Chase trigger asset missing')
    count=struct.unpack_from('<I',chase,8)[0]
    assert count>278
    nodes=list(struct.unpack_from('<'+'I'*count,chase,12))+[len(chase)]
    start,end=nodes[278:280]
    assert struct.unpack_from('<HH',chase,start)==(6,0)
    command=((start+4+3)&~3)+4
    assert struct.unpack_from('<H',chase,command)[0]==305
    assert b'L3A1a_T\0' in chase[command:end]
    # L5A3 follows the Venom fight. Its room groups are authored visibility
    # changes, so keep the regression tied to the installed sewer trigger bank.
    for i in range(files):
        name,crc,compressed,pos,size,packed=struct.unpack_from('<32sIiIII',data,off+i*52)
        if name.split(b'\0')[0].lower()==b'l5a3_t.trg':
            sewer=data[pos:pos+packed]
            sewer=zlib.decompress(sewer) if compressed==2 else sewer
            break
    else:raise AssertionError('Sewer trigger asset missing')
    count=struct.unpack_from('<I',sewer,8)[0]
    assert count>225
    nodes=list(struct.unpack_from('<'+'I'*count,sewer,12))+[len(sewer)]
    for node in (223,224,225):
        start,end=nodes[node:node+2]
        kind,links=struct.unpack_from('<HH',sewer,start);assert kind==6
        begin=((start+4+2*links+3)&~3)+4
        words=struct.unpack_from('<'+'H'*((end-begin)//2),sewer,begin)
        lines.append('static unsigned short sewer%d[]={%s};'%(node,','.join(map(str,words))))
    (work/'bank_fixture.h').write_text('\n'.join(lines+extra))
