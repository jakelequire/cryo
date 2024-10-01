; ModuleID = 'Std'
source_filename = "/home/phock/Programming/apps/cryo/cryo/std/Math.cryo"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

define i32 @add(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  %b.addr = alloca i32, i32 %b, align 4
  %addtmp = add i32 %a, %b
  ret i32 %addtmp
}

define i32 @subtract(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  %b.addr = alloca i32, i32 %b, align 4
  %subtmp = sub i32 %a, %b
  ret i32 %subtmp
}

define i32 @multiply(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  %b.addr = alloca i32, i32 %b, align 4
  %multmp = mul i32 %a, %b
  ret i32 %multmp
}

define i32 @divide(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  %b.addr = alloca i32, i32 %b, align 4
  %divtmp = sdiv i32 %a, %b
  ret i32 %divtmp
}
