; ModuleID = 'std'
source_filename = "/home/phock/Programming/apps/cryo/cryo/std/IO.cryo"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

declare void @printInt(i32)

declare void @printStr(ptr)

declare i32 @strLength(ptr)

define void @print() {
entry:
  ret void
}

define void @getStringLength() {
entry:
  %testStrLength = alloca i32, align 4
  %str = alloca [8 x i8], align 1
  store [8 x i8] c"strTest\00", ptr %str, align 1
  %0 = call i32 @strLength(ptr %str)
  store i32 %0, ptr %testStrLength, align 4
  %str1 = alloca [15 x i8], align 1
  store [15 x i8] c"String length:\00", ptr %str1, align 1
  call void @printStr(ptr %str1)
  call void @printInt(i32 %0)
  ret void
}
