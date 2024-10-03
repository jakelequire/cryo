; ModuleID = 'Test'
source_filename = "/workspaces/cryo/tests/test.cryo"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

define void @test(i32 %a) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  br label %ifCondition

ifCondition:                                      ; preds = %entry
  %ifCondition1 = icmp slt i32 %a, 5
  br i1 %ifCondition1, label %thenBlock, label %elseBlock

thenBlock:                                        ; preds = %ifCondition
  call void @test(i32 %a)
  br label %mergeBlock

elseBlock:                                        ; preds = %ifCondition
  ret void

mergeBlock:                                       ; preds = %thenBlock
  ret void
}

define void @main() {
entry:
  ret void
}
