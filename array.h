#pragma once

// Maintain a dynamic array with realloc()/free():
//  - start with ptr=NULL and len=0.
//  - pass push_back() the current array length, pre-growth:  push_back(ptr, len++)
//  - pass drop_back() the  *new*  array length, post-shrink: drop_back(ptr, --len)
//  - drop_back() will free() and return NULL when len=0.

void* push_back_(void *ptr, int len, int sizeofT);
void* drop_back_(void *ptr, int len, int sizeofT);

#define push_back(ptr,len) push_back_(ptr,len,sizeof *(ptr))
#define drop_back(ptr,len) drop_back_(ptr,len,sizeof *(ptr))
