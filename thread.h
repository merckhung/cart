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
 * Thread header support
 */

#ifndef CART_THREAD_H_
#define CART_THREAD_H_

#include <pthread.h>

#include "jni.h"
#include "jni_env_ext.h"

#include "macros.h"

#define kNumRosAllocThreadLocalSizeBrackets 34
#define kMaxCheckpoints 3

enum LockLevel {
  kLoggingLock = 0,
  kMemMapsLock,
  kSwapMutexesLock,
  kUnexpectedSignalLock,
  kThreadSuspendCountLock,
  kAbortLock,
  kJdwpSocketLock,
  kReferenceQueueSoftReferencesLock,
  kReferenceQueuePhantomReferencesLock,
  kReferenceQueueFinalizerReferencesLock,
  kReferenceQueueWeakReferencesLock,
  kReferenceQueueClearedReferencesLock,
  kReferenceProcessorLock,
  kRosAllocGlobalLock,
  kRosAllocBracketLock,
  kRosAllocBulkFreeLock,
  kAllocSpaceLock,
  kDexFileMethodInlinerLock,
  kDexFileToMethodInlinerMapLock,
  kMarkSweepMarkStackLock,
  kTransactionLogLock,
  kInternTableLock,
  kOatFileSecondaryLookupLock,
  kDefaultMutexLevel,
  kMarkSweepLargeObjectLock,
  kPinTableLock,
  kLoadLibraryLock,
  kJdwpObjectRegistryLock,
  kModifyLdtLock,
  kAllocatedThreadIdsLock,
  kMonitorPoolLock,
  kClassLinkerClassesLock,
  kBreakpointLock,
  kMonitorLock,
  kMonitorListLock,
  kThreadListLock,
  kBreakpointInvokeLock,
  kAllocTrackerLock,
  kDeoptimizationLock,
  kProfilerLock,
  kJdwpEventListLock,
  kJdwpAttachLock,
  kJdwpStartLock,
  kArtdbgStartLock,
  kRuntimeShutdownLock,
  kTraceLock,
  kHeapBitmapLock,
  kMutatorLock,
  kInstrumentEntrypointsLock,
  kThreadListSuspendThreadLock,
  kZygoteCreationLock,

  kLockLevelCount  // Must come last.
};

// We have no control over the size of 'bool', but want our boolean fields
// to be 4-byte quantities.
typedef uint32_t bool32_t;
typedef uint8_t byte;

typedef struct PACKED {
  volatile uint16_t flags;
  volatile uint16_t state;
} StateAndFlags_t;

typedef struct PACKED {
  StateAndFlags_t state_and_flags;

  // A non-zero value is used to tell the current thread to enter a safe point
  // at the next poll.
  int suspend_count;

  // How much of 'suspend_count_' is by request of the debugger, used to set things right
  // when the debugger detaches. Must be <= suspend_count_.
  int debug_suspend_count;

  // Thin lock thread id. This is a small integer used by the thin lock implementation.
  // This is not to be confused with the native thread's tid, nor is it the value returned
  // by java.lang.Thread.getId --- this is a distinct value, used only for locking. One
  // important difference between this id and the ids visible to managed code is that these
  // ones get reused (to ensure that they fit in the number of bits available).
  uint32_t thin_lock_thread_id;

  // System thread id.
  uint32_t tid;

  // Is the thread a daemon?
  bool32_t daemon;

  // A boolean telling us whether we're recursively throwing OOME.
  bool32_t throwing_OutOfMemoryError;

  // A positive value implies we're in a region where thread suspension isn't expected.
  uint32_t no_thread_suspension;

  // How many times has our pthread key's destructor been called?
  uint32_t thread_exit_check_count;

  // When true this field indicates that the exception associated with this thread has already
  // been reported to instrumentation.
  bool32_t is_exception_reported_to_instrumentation_;

  // True if signal is being handled by this thread.
  bool32_t handling_signal_;

  // Padding to make the size aligned to 8.  Remove this if we add another 32 bit field.
  int32_t padding_;
} tls_32bit_sized_values_t;

typedef struct PACKED {
  uint8_t z;
  int8_t b;
  uint16_t c;
  int16_t s;
  int32_t i;
  int64_t j;
  float f;
  double d;
  void* l;  // mirror::Object* l;
} JValue_t;

typedef struct PACKED {
  // Number of objects allocated.
  uint64_t allocated_objects;
  // Cumulative size of all objects allocated.
  uint64_t allocated_bytes;

  // Number of objects freed.
  uint64_t freed_objects;
  // Cumulative size of all freed objects.
  uint64_t freed_bytes;

  // Number of times an allocation triggered a GC.
  uint64_t gc_for_alloc_count;

  // Number of initialized classes.
  uint64_t class_init_count;
  // Cumulative time spent in class initialization.
  uint64_t class_init_time_ns;
} RuntimeStats_t;

typedef struct PACKED {
  // The clock base used for tracing.
  uint64_t trace_clock_base;

  // Return value used by deoptimization.
  JValue_t deoptimization_return_value;

  RuntimeStats_t stats;
} tls_64bit_sized_values_t;

typedef struct PACKED ManagedStack {
  struct ManagedStack* link_;
  void* top_shadow_frame_;  // ShadowFrame* top_shadow_frame_;
  void* top_quick_frame_;  // StackReference<mirror::ArtMethod>* top_quick_frame_;
  uintptr_t top_quick_frame_pc_;
} ManagedStack_t;

typedef struct PACKED {
  void* pInterpreterToInterpreterBridge;
  void* pInterpreterToCompiledCodeBridge;
} InterpreterEntryPoints_t;

typedef struct PACKED {
  // Invocation
  void (*pPortableImtConflictTrampoline)(void*);
  void (*pPortableResolutionTrampoline)(void*);
  void (*pPortableToInterpreterBridge)(void*);
} PortableEntryPoints_t;

typedef struct PACKED {
  // Called when the JNI method isn't registered.
  void* (*pDlsymLookup)(JNIEnv* env, jobject);
} JniEntryPoints_t;

typedef struct PACKED {
  // The 'this' reference of the throwing method.
  void* this_object_;  // mirror::Object* this_object_;
  // The throwing method.
  void* method_;  // mirror::ArtMethod* method_;
  // The instruction within the throwing method.
  uint32_t dex_pc_;
  // Ensure 8byte alignment on 64bit.
#ifdef __LP64__
  uint32_t pad_;
#endif
} ThrowLocation_t;

typedef struct PACKED {
  void* (*AllocArray)(void*, uint32_t);
  void* (*AllocArrayResolved)(void*, uint32_t);
  void* (*AllocArrayWithAccessCheck)(void*, uint32_t);
  void* (*AllocObject)(void*, uint32_t);
  void* (*AllocObjectResolved)(void*, uint32_t);
  void* (*AllocObjectInitialized)(void*, uint32_t);
  void* (*AllocObjectWithAccessCheck)(void*, uint32_t);
  void* (*CheckAndAllocArray)(void*, uint32_t);
  void* (*CheckAndAllocArrayWithAccessCheck)(void*, uint32_t);

  void* (*InstanceofNonTrivial)(void*, uint32_t);
  void* (*CheckCast)(void*, uint32_t);

  void* (*InitializeStaticStorage)(void*, uint32_t);
  void* (*InitializeTypeAndVerifyAccess)(void*, uint32_t);
  void* (*InitializeType)(void*, uint32_t);
  void* (*ResolveString)(void*, uint32_t);

  void* (*Set32Instance)(void*, uint32_t);
  void* (*Set32Static)(void*, uint32_t);
  void* (*Set64Instance)(void*, uint32_t);
  void* (*Set64Static)(void*, uint32_t);
  void* (*SetObjInstance)(void*, uint32_t);
  void* (*SetObjStatic)(void*, uint32_t);
  void* (*Get32Instance)(void*, uint32_t);
  void* (*Get32Static)(void*, uint32_t);
  void* (*Get64Instance)(void*, uint32_t);
  void* (*Get64Static)(void*, uint32_t);
  void* (*GetObjInstance)(void*, uint32_t);
  void* (*GetObjStatic)(void*, uint32_t);

  void* (*AputObjectWithNullAndBoundCheck)(void*, uint32_t);
  void* (*AputObjectWithBoundCheck)(void*, uint32_t);
  void* (*AputObject)(void*, uint32_t);
  void* (*HandleFillArrayData)(void*, uint32_t);

  void* (*JniMethodStart)(void*, uint32_t);
  void* (*JniMethodStartSynchronized)(void*, uint32_t);
  void* (*JniMethodEnd)(void*, uint32_t);
  void* (*JniMethodEndSynchronized)(void*, uint32_t);
  void* (*JniMethodEndWithReference)(void*, uint32_t);
  void* (*JniMethodEndWithReferenceSynchronized)(void*, uint32_t);
  void* (*QuickGenericJniTrampoline)(void*, uint32_t);

  void* (*LockObject)(void*, uint32_t);
  void* (*UnlockObject)(void*, uint32_t);

  void* (*CmpgDouble)(void*, uint32_t);
  void* (*CmpgFloat)(void*, uint32_t);
  void* (*CmplDouble)(void*, uint32_t);
  void* (*CmplFloat)(void*, uint32_t);
  void* (*Fmod)(void*, uint32_t);
  void* (*L2d)(void*, uint32_t);
  void* (*Fmodf)(void*, uint32_t);
  void* (*L2f)(void*, uint32_t);
  void* (*D2iz)(void*, uint32_t);
  void* (*F2iz)(void*, uint32_t);
  void* (*Idivmod)(void*, uint32_t);
  void* (*D2l)(void*, uint32_t);
  void* (*F2l)(void*, uint32_t);
  void* (*Ldiv)(void*, uint32_t);
  void* (*Lmod)(void*, uint32_t);
  void* (*Lmul)(void*, uint32_t);
  void* (*ShlLong)(void*, uint32_t);
  void* (*ShrLong)(void*, uint32_t);
  void* (*UshrLong)(void*, uint32_t);

  void* (*IndexOf)(void*, uint32_t);
  void* (*StringCompareTo)(void*, uint32_t);
  void* (*Memcpy)(void*, uint32_t);

  void* (*QuickImtConflictTrampoline)(void*, uint32_t);
  void* (*QuickResolutionTrampoline)(void*, uint32_t);
  void* (*QuickToInterpreterBridge)(void*, uint32_t);
  void* (*InvokeDirectTrampolineWithAccessCheck)(void*, uint32_t);
  void* (*InvokeInterfaceTrampolineWithAccessCheck)(void*, uint32_t);
  void* (*InvokeStaticTrampolineWithAccessCheck)(void*, uint32_t);
  void* (*InvokeSuperTrampolineWithAccessCheck)(void*, uint32_t);
  void* (*InvokeVirtualTrampolineWithAccessCheck)(void*, uint32_t);

  void (*TestSuspend)();

  void* (*DeliverException)(void*, uint32_t);
  void* (*ThrowArrayBounds)(void*, uint32_t);
  void* (*ThrowDivZero)(void*, uint32_t);
  void* (*ThrowNoSuchMethod)(void*, uint32_t);
  void* (*ThrowNullPointer)(void*, uint32_t);
  void* (*ThrowStackOverflow)(void*, uint32_t);

  void* (*A64Load)(void*, uint32_t);
  void* (*A64Store)(void*, uint32_t);
} QuickEntryPoints_t;

typedef struct PACKED {
  // The biased card table, see CardTable for details.
  byte* card_table;

  // The pending exception or NULL.
  void* exception;  // mirror::Throwable* exception;

  // The end of this thread's stack. This is the lowest safely-addressable address on the stack.
  // We leave extra space so there's room for the code that throws StackOverflowError.
  byte* stack_end;

  // The top of the managed stack often manipulated directly by compiler generated code.
  ManagedStack_t managed_stack;

  // In certain modes, setting this to 0 will trigger a SEGV and thus a suspend check.  It is
  // normally set to the address of itself.
  uintptr_t* suspend_trigger;

  // Every thread may have an associated JNI environment
  cart::JNIEnvExt* jni_env;

  // Initialized to "this". On certain architectures (such as x86) reading off of Thread::Current
  // is easy but getting the address of Thread::Current is hard. This field can be read off of
  // Thread::Current to give the address.
  void* self;  // Thread* self;

  // Our managed peer (an instance of java.lang.Thread). The jobject version is used during thread
  // start up, until the thread is registered and the local opeer_ is used.
  void* opeer;  // mirror::Object* opeer;
  void* jpeer;  // jobject jpeer;

  // The "lowest addressable byte" of the stack.
  byte* stack_begin;

  // Size of the stack.
  size_t stack_size;

  // The location the current exception was thrown from.
  ThrowLocation_t throw_location;

  // Pointer to previous stack trace captured by sampling profiler.
  // std::vector<mirror::ArtMethod*>* stack_trace_sample;
  void* stack_trace_sample;

  // The next thread in the wait set this thread is part of or NULL if not waiting.
  void* wait_next;  // Thread* wait_next;

  // If we're blocked in MonitorEnter, this is the object we're trying to lock.
  void* monitor_enter_object;  // mirror::Object* monitor_enter_object;

  // Top of linked list of handle scopes or nullptr for none.
  void* top_handle_scope;  // HandleScope* top_handle_scope;

  // Needed to get the right ClassLoader in JNI_OnLoad, but also
  // useful for testing.
  void* class_loader_override;  // mirror::ClassLoader* class_loader_override;

  // Thread local, lazily allocated, long jump context. Used to deliver exceptions.
  void* long_jump_context;  // Context* long_jump_context;

  // Additional stack used by method instrumentation to store method and return pc values.
  // Stored as a pointer since std::deque is not PACKED.
  // std::deque<instrumentation::InstrumentationStackFrame>* instrumentation_stack;
  void* instrumentation_stack;

  // JDWP invoke-during-breakpoint support.
  void* debug_invoke_req;  // DebugInvokeReq* debug_invoke_req;

  // JDWP single-stepping support.
  void* single_step_control;  // SingleStepControl* single_step_control;

  // Shadow frame stack that is used temporarily during the deoptimization of a method.
  void* deoptimization_shadow_frame;  // ShadowFrame* deoptimization_shadow_frame;

  // Shadow frame stack that is currently under construction but not yet on the stack
  void* shadow_frame_under_construction;  // ShadowFrame* shadow_frame_under_construction;

  // A cached copy of the java.lang.Thread's name.
  void* name;  // std::string* name;

  // A cached pthread_t for the pthread underlying this Thread*.
  pthread_t pthread_self;

  // If no_thread_suspension_ is > 0, what is causing that assertion.
  const char* last_no_thread_suspension_cause;

  // Pending checkpoint function or NULL if non-pending. Installation guarding by
  // Locks::thread_suspend_count_lock_.
  void* checkpoint_functions[kMaxCheckpoints];  // Closure* checkpoint_functions[kMaxCheckpoints];

  // Entrypoint function pointers.
  // TODO: move this to more of a global offset table model to avoid per-thread duplication.
  InterpreterEntryPoints_t interpreter_entrypoints;
  JniEntryPoints_t jni_entrypoints;
  PortableEntryPoints_t portable_entrypoints;
  QuickEntryPoints_t quick_entrypoints;

  // Thread-local allocation pointer.
  byte* thread_local_start;
  byte* thread_local_pos;
  byte* thread_local_end;
  size_t thread_local_objects;

  // There are RosAlloc::kNumThreadLocalSizeBrackets thread-local size brackets per thread.
  void* rosalloc_runs[kNumRosAllocThreadLocalSizeBrackets];

  // Thread-local allocation stack data/routines.
  void** thread_local_alloc_stack_top;  // mirror::Object** thread_local_alloc_stack_top;
  void** thread_local_alloc_stack_end;  // mirror::Object** thread_local_alloc_stack_end;

  // Support for Mutex lock hierarchy bug detection.
  void* held_mutexes[kLockLevelCount];  // BaseMutex* held_mutexes[kLockLevelCount];

  // Recorded thread state for nested signals.
  void* nested_signal_state;  // jmp_buf* nested_signal_state;
} tls_ptr_sized_values_t;

#endif  // CART_THREAD_H_

