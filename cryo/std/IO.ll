; ModuleID = 'std'
source_filename = "/workspaces/cryo/cryo/std/IO.cryo"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

declare void @printInt(i32)

declare void @printStr(ptr)

declare i32 @strLength(ptr)
