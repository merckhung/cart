/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * List header support
 */

#ifndef CART_LIST_H_
#define CART_LIST_H_

typedef union _CommonLinkList {
  union _CommonLinkList *Next;
} CommonLinkList;

typedef union _DoublyLinkList {
  union _CommonLinkList *Next;
  union _CommonLinkList *Prev;
} DoublyLinkList;

#define LlAddObjectToHead(HEAD, PLL)  _LlAddObjectToHead((CommonLinkList **)&HEAD, (CommonLinkList *)PLL)
#define LlAddObjectToTail(HEAD, PLL)  _LlAddObjectToTail((CommonLinkList *)HEAD, (CommonLinkList *)PLL)
#define LlInsertObject(TARGET, PLL)   _LlInsertObject((CommonLinkList *)TARGET, (CommonLinkList *)PLL)
#define LlGetObjectFromHead(HEAD)     _LlGetObjectFromHead((CommonLinkList **)&HEAD)
#define LlGetTailObject(HEAD)         _LlGetTailObject((CommonLinkList *)HEAD)
#define LlRemoveObject(HEAD, TARGET)  _LlRemoveObject((CommonLinkList **)&HEAD, (CommonLinkList *)TARGET)
#define LlFreeAllObject(HEAD)         _LlFreeAllObject((CommonLinkList *)HEAD)

#define LlMalloc(HEAD, POINTER, COUNTER, DATATYPE) { \
  if (COUNTER) { \
    POINTER = &((*POINTER)->Next); \
  } else { \
    POINTER = HEAD; \
  } \
  *POINTER = (DATATYPE*)malloc(sizeof(DATATYPE)); \
  memset(*POINTER, 0, sizeof(DATATYPE)); \
}

#ifdef CART_DEBUG
#define LlDebugPrint(HEAD, POINTER, FORMAT, ARGS...) { \
  for (POINTER = HEAD; POINTER; POINTER = POINTER->Next) { \
    pdbg(FORMAT, ##ARGS); \
  } \
}
#else
#define LlDebugPrint(HEAD, POINTER, FORMAT, ARGS...)
#endif

void _LlAddObjectToHead(CommonLinkList **Phead, CommonLinkList *Pll);
void _LlAddObjectToTail(CommonLinkList *Head, CommonLinkList *Pll);
void _LlInsertObject(CommonLinkList *Target, CommonLinkList *Pll);
CommonLinkList *_LlGetObjectFromHead(CommonLinkList **Phead);
CommonLinkList *_LlGetObjectFromTail(CommonLinkList *Head);
CommonLinkList *_LlGetTailObject(CommonLinkList *Head);
void _LlRemoveObject(CommonLinkList **Phead, CommonLinkList *Target);
void _LlFreeAllObject(CommonLinkList *Head);

#endif  // CART_LIST_H_
