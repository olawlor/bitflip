/* A tiny UNIXy utility to check for RAM errors at runtime. 

Dr. Orion Lawlor, lawlor@alaska.edu, 2020-09-26 (Public Domain)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include <unistd.h> // for usleep()

FILE *error_log;

class RAMcheck {
private:
// This is the datatype we used for checking
    typedef unsigned char check_t;
    enum { CHECK_PER_MB=1024*1024/sizeof(check_t) };

// We check a fixed prime-repeating pattern of bits
    enum { PATTERN_SIZE=257 }; // as suggested by http://www.ganssle.com/item/how-to-test-ram.htm
    const static check_t pattern[PATTERN_SIZE];
    
// This is the actual buffer storing the data to check
    size_t buffer_size;
    std::vector<check_t> buffer;

// We check the buffer in small chunks (to spread out our CPU usage)
    size_t chunk_start;
    size_t chunk_index;
    enum { CHUNK_SIZE=1024*1024/sizeof(check_t)/PATTERN_SIZE*PATTERN_SIZE };
    
    bool pass_errors;

public:
    // Set up a RAM checker for this many megs
    RAMcheck(long mb) 
        :buffer_size(mb*CHECK_PER_MB), buffer(buffer_size) 
    {
        fill();
    }
    
    // Fill our buffer with the test pattern
    void fill() {
        size_t index=0;
        for (size_t b=0;b<buffer_size;b++) {
            buffer[b]=pattern[index];
            index++;
            if (index>=PATTERN_SIZE) index=0;
        }
        chunk_start=0;
        chunk_index=0;
        pass_errors=false;
    }
    
    // Check the next chunk of data
    void check_chunk() {
        size_t end=chunk_start+CHUNK_SIZE;
        if (end>buffer_size) end=buffer_size;
        size_t index=0; //<- because chunk_size is a multiple of pattern_size
        for (size_t b=chunk_start;b<end;b+=PATTERN_SIZE) {
            size_t n=PATTERN_SIZE;
            if (b+n>end) n=end-b;
            if (0!=memcmp(&buffer[b],pattern,n*sizeof(check_t)))
                found_mismatch_block(b,n);
        }
        
        // Advance the chunk pointer
        chunk_start=end;
        if (chunk_start>=buffer_size) {
            fprintf(stderr,"Pass %s\n",pass_errors?"FAIL":"OK");
            fflush(stdout);
            if (pass_errors) fill(); // <- so we don't re-report errors
            chunk_start=0;
            chunk_index=0;
        }
    }
    
    // Mismatch detected at this block (rare)
    void found_mismatch_block(size_t buffer_start,size_t block_len); 
    
    // Mismatch detected at this byte (rare)
    virtual void found_mismatch(size_t buffer_index,size_t pattern_index) 
    {
        size_t b=buffer_index;
        int should=pattern[pattern_index];
        int had=buffer[b];
        int flip=had^should;
        
        fprintf(stderr,"RAM MISMATCH DETECTED: Index %ld should contain %02x actually had %02x (flip %02x)\n",
            b, should, had, flip);
        fflush(stderr);
        
        if (error_log) {
            fprintf(error_log,"RAM MISMATCH DETECTED: Index %ld should contain %02x actually had %02x (flip %02x)\n",
                b, should, had, flip);
            fflush(error_log);
        }
        
        pass_errors=true;
        // Keep running, to detect more errors
    }
    
};

// Declared down here, so it's not inline
void RAMcheck::found_mismatch_block(size_t buffer_start,size_t block_len)
{
    fprintf(stderr,"Mismatch found in block starting at %ld size %ld\n",buffer_start,block_len);
    for (size_t index=0;index<block_len;index++)
    {
        size_t b=buffer_start+index;
        if (buffer[b]!=pattern[index])
            found_mismatch(b,index);
    }
}
    
const RAMcheck::check_t RAMcheck::pattern[RAMcheck::PATTERN_SIZE]=
{
  0x55,0x55,0xaa,0xaa,0x55,0x55,0xaa,0xaa, // fencepost patterns (8)
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // all 1's (8)
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // all 0's (8)
  
 // Random data from:  dd if=/dev/urandom bs=233 count=1 | xxd -i 
  0x98, 0xcc, 0x95, 0x4f, 0x96, 0xe9, 0x5c, 0x27, 0xab, 0xa9, 0xee, 0x16,
  0xad, 0x9e, 0x61, 0xf2, 0x94, 0x1d, 0x83, 0x19, 0x9a, 0x23, 0x0a, 0x31,
  0xec, 0x30, 0x43, 0xdf, 0xdf, 0x19, 0x8c, 0x40, 0x73, 0x73, 0xef, 0x3a,
  0x70, 0xf4, 0x58, 0xa3, 0x67, 0x95, 0xe6, 0x5a, 0x15, 0xb1, 0x13, 0x00,
  0x7d, 0x2c, 0x51, 0xe1, 0xc4, 0x00, 0xc4, 0xe7, 0x15, 0x4d, 0xaf, 0x85,
  0x1a, 0x5e, 0x21, 0x0a, 0xa1, 0x8d, 0xdc, 0xae, 0x66, 0xf9, 0x5e, 0xc7,
  0x25, 0xab, 0x7a, 0xee, 0x2d, 0x7a, 0x0f, 0x33, 0x43, 0x53, 0x21, 0xe6,
  0xd4, 0x4e, 0x0f, 0x8b, 0x6e, 0xa6, 0x67, 0x98, 0x74, 0x80, 0x0e, 0x82,
  0xdf, 0xb6, 0x4a, 0xc9, 0xe2, 0x49, 0x45, 0x6c, 0xe6, 0xc6, 0x64, 0x73,
  0xcd, 0xa8, 0xe3, 0xe5, 0x86, 0x77, 0x95, 0xe6, 0x7d, 0x33, 0x71, 0x2f,
  0xf9, 0x13, 0xd6, 0xd2, 0x4e, 0xbe, 0x78, 0x4d, 0x52, 0xcf, 0x83, 0xf6,
  0xb3, 0xdd, 0x94, 0xbc, 0xff, 0x88, 0xcd, 0x72, 0xa5, 0x72, 0x55, 0x0a,
  0x4d, 0x76, 0x49, 0xf8, 0x96, 0x86, 0x2c, 0x53, 0x87, 0x70, 0x44, 0x7b,
  0x14, 0x4f, 0x0d, 0xd1, 0x6f, 0x30, 0x88, 0x8d, 0xe9, 0xf0, 0xf8, 0x4a,
  0xe4, 0x6c, 0x82, 0xa3, 0x24, 0xdb, 0x65, 0x4d, 0x1e, 0xe6, 0xab, 0x0c,
  0xab, 0x42, 0xaf, 0xc8, 0xfc, 0xab, 0xd1, 0x15, 0x05, 0xdc, 0x22, 0xbf,
  0x79, 0x33, 0x41, 0x62, 0x73, 0x6e, 0xea, 0x0e, 0xb5, 0xa3, 0xdf, 0x84,
  0x34, 0xdb, 0x70, 0xdd, 0x3e, 0x48, 0x7a, 0xc8, 0x68, 0x98, 0x3d, 0x32,
  0x40, 0x10, 0x72, 0x43, 0xc8, 0x93, 0xdc, 0xfc, 0x43, 0x60, 0x49, 0xdb,
  0xd7, 0x15, 0x41, 0x93, 0x60
};

int main(int argc,char *argv[])
{
    if (argc<=1) {
        printf("Usage: bitflip <megs of memory to test>\n");
    }
    long mb=atoi(argv[1]);
    
    error_log=fopen("/tmp/bitflip.log","a");
    if (error_log) {
        fprintf(error_log,"Started bitflip, testing %ld MB of RAM\n",mb);
        fflush(error_log);
    }
    
    RAMcheck checker(mb);
    printf("Initialized %ld megs of memory.  Test running...\n",mb);
    fflush(stdout);
    while (1) {
        checker.check_chunk();
        usleep(100*1000); // 10MB/sec check rate; results in <1% usage of 1 core
    }
}


