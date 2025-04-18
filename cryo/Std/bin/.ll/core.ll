; ModuleID = 'core'
source_filename = "core"

%struct.Int = type { i32 }
%struct.String = type { i32, i32, ptr, i1, i1 }
%VA_ARGS = type { i32, ptr, ptr }

@Int = external global %struct.Int
@String = external global %struct.String

; Function Attrs: nounwind
declare i32 @strlen_export(ptr) #0

; Function Attrs: nounwind
declare ptr @strncat_export(ptr, ptr, i32) #0

; Function Attrs: nounwind
declare i32 @strnlen_export(ptr, i32) #0

; Function Attrs: nounwind
declare i32 @strncmp_export(ptr, ptr, i32) #0

; Function Attrs: nounwind
define ptr @struct.Int.ctor(ptr %self, i32 %0) #0 {
entry:
  %self.alloc = alloca %struct.Int, align 8
  store ptr %self, ptr %self.alloc, align 8
  %__val__ = alloca ptr, align 8
  store i32 %0, ptr %__val__, align 4
  %val = getelementptr inbounds %struct.Int, ptr %self.alloc, i32 0, i32 0
  store ptr %__val__, ptr %val, align 8
  ret ptr %self.alloc
}

; Function Attrs: nounwind
define ptr @struct.String.ctor(ptr %self, ptr %0) #0 {
entry:
  %self.alloc = alloca %struct.String, align 8
  store ptr %self, ptr %self.alloc, align 8
  %__string__ = alloca ptr, align 8
  store ptr %0, ptr %__string__, align 8
  %val = getelementptr inbounds %struct.String, ptr %self.alloc, i32 0, i32 0
  store ptr %__string__, ptr %val, align 8
  ret ptr %self.alloc
}

; Function Attrs: nounwind
declare i32 @printf_export(ptr, %VA_ARGS) #0

; Function Attrs: nounwind
declare i32 @scanf_export(ptr, %VA_ARGS) #0

; Function Attrs: nounwind
declare void @printI32_export(i32) #0

; Function Attrs: nounwind
declare void @printString_export(%struct.String) #0

; Function Attrs: nounwind
declare void @sys_exit(%struct.Int) #0

; Function Attrs: nounwind
declare %struct.Int @sys_read(%struct.Int, ptr, %struct.Int) #0

; Function Attrs: nounwind
declare %struct.Int @sys_write(%struct.Int, ptr, %struct.Int) #0

; Function Attrs: nounwind
declare ptr @mmap_export(ptr, %struct.Int, %struct.Int, %struct.Int, %struct.Int, %struct.Int) #0

; Function Attrs: nounwind
declare %struct.Int @munmap_export(ptr, %struct.Int) #0

; Function Attrs: nounwind
declare ptr @malloc_export(%struct.Int) #0

; Function Attrs: nounwind
declare %struct.Int @free_export(ptr) #0

; Function Attrs: nounwind
declare ptr @memcpy_export(ptr, ptr, %struct.Int) #0

; Function Attrs: nounwind
declare ptr @memmove_export(ptr, ptr, %struct.Int) #0

; Function Attrs: nounwind
declare %struct.Int @memcmp_export(ptr, ptr, %struct.Int) #0

; Function Attrs: nounwind
declare ptr @memset_export(ptr, %struct.Int, %struct.Int) #0

define void @printInt(%struct.Int %value) {
entry:
  %value.alloca = alloca %struct.Int, align 8
  store %struct.Int %value, ptr %value.alloca, align 4
  %strAlloc = alloca [4 x i8], align 1
  store [4 x i8] c"%d\0A\00", ptr %strAlloc, align 1
  %fc_idx_0. = ptrtoint ptr %strAlloc to i8
  %printf_export = call i32 @printf_export(i8 %fc_idx_0.)
  %calltmp = alloca i32, align 4
  store i32 %printf_export, ptr %calltmp, align 4
  ret void
}

define void @printString(%struct.String %value) {
entry:
  %value.alloca = alloca %struct.String, align 8
  store %struct.String %value, ptr %value.alloca, align 8
  %fc_idx_0.value.alloca.load = load %struct.String, ptr %value.alloca, align 8
  call void @printString_export(%struct.String %fc_idx_0.value.alloca.load)
  ret void
}

define void @printI32(i32 %value) {
entry:
  %value.alloca = alloca i32, align 4
  store i32 %value, ptr %value.alloca, align 4
  %strAlloc = alloca [4 x i8], align 1
  store [4 x i8] c"%d\0A\00", ptr %strAlloc, align 1
  %fc_idx_0. = ptrtoint ptr %strAlloc to i8
  %fc_idx_1.value.alloca.load = load i32, ptr %value.alloca, align 4
  %printf_export = call i32 @printf_export(i8 %fc_idx_0., i32 %fc_idx_1.value.alloca.load)
  %calltmp = alloca i32, align 4
  store i32 %printf_export, ptr %calltmp, align 4
  ret void
}

define void @printStr(%struct.String %value) {
entry:
  %value.alloca = alloca %struct.String, align 8
  store %struct.String %value, ptr %value.alloca, align 8
  %strAlloc = alloca [4 x i8], align 1
  store [4 x i8] c"%s\0A\00", ptr %strAlloc, align 1
  %fc_idx_0. = ptrtoint ptr %strAlloc to i8
  %printf_export = call i32 @printf_export(i8 %fc_idx_0.)
  %calltmp = alloca i32, align 4
  store i32 %printf_export, ptr %calltmp, align 4
  ret void
}

define void @printChar(i8 %value) {
entry:
  %value.alloca = alloca i8, align 1
  store i8 %value, ptr %value.alloca, align 1
  %strAlloc = alloca [4 x i8], align 1
  store [4 x i8] c"%c\0A\00", ptr %strAlloc, align 1
  %fc_idx_0. = ptrtoint ptr %strAlloc to i8
  %fc_idx_1.value.alloca.load = load i8, ptr %value.alloca, align 1
  %printf_export = call i32 @printf_export(i8 %fc_idx_0., i8 %fc_idx_1.value.alloca.load)
  %calltmp = alloca i32, align 4
  store i32 %printf_export, ptr %calltmp, align 4
  ret void
}

attributes #0 = { nounwind }
