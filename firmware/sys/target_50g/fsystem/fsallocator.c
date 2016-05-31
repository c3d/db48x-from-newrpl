#define CHUNK_SIZE  ((2016/8)*2+3)
#define MAX_CHUNKS  8
#define BLOCK_WORDS ((CHUNK_SIZE)>>5)
// *** DEBUG ONY ***
// STATIC CHUNKS OF MEMORY

unsigned int st_chunks[MAX_CHUNKS][CHUNK_SIZE];
int used_chunks=0;

// *** END DEBUG ONLY



struct {
    unsigned int *chunks[MAX_CHUNKS];
    unsigned int chunk_bmp[MAX_CHUNKS];
    unsigned int chunk_1stblk[MAX_CHUNKS];
    int last_chunk;
} SimpAllocData;




void init_simpalloc()
{
    int f;
    for(f=0;f<MAX_CHUNKS;++f)
    {
        SimpAllocData.chunks[f]=0;
        SimpAllocData.chunk_bmp[f]=~0;
        SimpAllocData.chunk_1stblk[f]=0;
    }
    SimpAllocData.last_chunk=0;
}

// ADD A NEW CHUNK TO THE POOL
int need_chunk()
{
    // TODO: CONNECT THIS WITH ANOTHER ALLOCATOR TO GET PAGES
    // FOR NOW GET A STATIC CHUNK
    int f;
    for(f=0;f<MAX_CHUNKS;++f)
    {
        if(SimpAllocData.chunks[f]==0) {
            SimpAllocData.chunks[f]=st_chunks[used_chunks];
            ++used_chunks;
            break;
    }
    }
    if(f==MAX_CHUNKS) return -1; // CAN'T ALLOCATE ANY MORE CHUNKS
    SimpAllocData.chunk_bmp[f]=0; // MARK ALL BLOCKS FREE
    SimpAllocData.chunk_1stblk[f]=0;
    return f;   // RETURN THE NUMBER OF THE NEW CHUNK
}

void release_chunk(int f)
{
    // TODO: CONNECT THIS WITH ANOTHER ALLOCATOR TO RELEASE PAGES
    // FOR NOW JUST CLEANUP THE STATIC CHUNKS
    SimpAllocData.chunks[f]=0;
    SimpAllocData.chunk_bmp[f]=~0;
    if(SimpAllocData.last_chunk==f) SimpAllocData.last_chunk=-1;
}


unsigned int *simpmalloc(int words)
{
    unsigned int mask=0,rotmask;
    int nblocks=0,f,ch,startch;
    while(words>0) {
        mask<<=1;
        mask|=1;
        ++nblocks;
        words-=BLOCK_WORDS;
    }
    // SELECT A CHUNK TO LOOK INTO
   startch=SimpAllocData.last_chunk;
    if(startch<0) startch=0;
    ch=startch;
    do {
    do {
    if(SimpAllocData.chunks[ch]) {

    for(f=0,rotmask=mask;f<33-nblocks;++f,rotmask<<=1) {
        // LOOK IN THE CURRENT CHUNK FIRST
        if((SimpAllocData.chunk_bmp[ch]&rotmask)==0) {
            // FOUND A BLOCK!
            SimpAllocData.chunk_bmp[ch]|=rotmask;
            SimpAllocData.chunk_1stblk[ch]|=rotmask;
            SimpAllocData.chunk_1stblk[ch]^=1<<f;
            SimpAllocData.last_chunk=ch;
            return SimpAllocData.chunks[ch]+BLOCK_WORDS*f;
        }
        }
    }
    ++ch;
    if(ch>=MAX_CHUNKS) ch=0;
    } while(ch!=startch);
    // NEED A NEW CHUNK!
        ch=need_chunk();

    } while(ch>=0);

    // FAILED TO ALLOCATE!
    return 0;

}

void simpfree(unsigned int *ptr)
{
    int ch,blk;
    unsigned int mask;

    for(ch=0;ch<MAX_CHUNKS;++ch)
    {
        if(SimpAllocData.chunks[ch]) {
        if( (ptr>=SimpAllocData.chunks[ch]) && (ptr-SimpAllocData.chunks[ch]<CHUNK_SIZE)) {
            // POINTER IS WITHIN THIS CHUNK
            blk=(ptr-SimpAllocData.chunks[ch])/BLOCK_WORDS;
            mask=1<<blk;
            SimpAllocData.chunk_bmp[ch]&=~mask; // FREE THE 1ST BLOCK
            mask<<=1;
            while(SimpAllocData.chunk_1stblk[ch]&mask) { SimpAllocData.chunk_bmp[ch]&=~mask; SimpAllocData.chunk_1stblk[ch]&=~mask; mask<<=1; }
            if(SimpAllocData.chunk_bmp[ch]==0) release_chunk(ch);
            return;
        }


        }
    }
    // POINTER WASN'T WITHIN THE ALLOCATED POOL!
    // DO NOTHING.

}
