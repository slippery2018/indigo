#ifndef __cf_storage__
#define __cf_storage__

#include "base_cpp/obj_array.h"
#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/tlscont.h"

#include "bingo_ptr.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace indigo;

namespace bingo
{
   class ByteBufferStorage
   {
   public:
      ByteBufferStorage (int block_size);

      static size_t create (BingoPtr<ByteBufferStorage> &cf_ptr, int block_size);

      static void load (BingoPtr<ByteBufferStorage> &cf_ptr, size_t offset);

      const byte * get (int idx, int &len);
      void add (const byte *data, int len, int idx);
      void remove (int idx);
      ~ByteBufferStorage();

   private:

      struct _Addr
      {
         unsigned long block_idx;
         unsigned long offset;
         long len;
      };

      int _block_size;
      int _free_pos;
      BingoArray< BingoPtr<byte> > _blocks;
      BingoArray<_Addr> _addresses;
   };
};

#endif /* __cf_storage__ */
