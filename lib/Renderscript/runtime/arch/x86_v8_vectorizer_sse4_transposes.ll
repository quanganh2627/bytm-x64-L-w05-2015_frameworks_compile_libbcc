declare <4 x i32> @_Z8shuffle2Dv4_iS_Dv4_j(<4 x i32> %x, <4 x i32> %y, <4 x i32> %mask) nounwind readnone
declare <2 x i64> @llvm.x86.sse2.psrl.dq(<2 x i64>, i32) nounwind readnone
declare <4 x float> @llvm.x86.sse41.blendps(<4 x float>, <4 x float>, i32) nounwind readnone

define void @__ocl_gather_transpose_char4x8(<4 x i8>* nocapture %pLoadAdd0, <4 x i8>* nocapture %pLoadAdd1, <4 x i8>* nocapture %pLoadAdd2, <4 x i8>* nocapture %pLoadAdd3, <4 x i8>* nocapture %pLoadAdd4, <4 x i8>* nocapture %pLoadAdd5, <4 x i8>* nocapture %pLoadAdd6, <4 x i8>* nocapture %pLoadAdd7, <8 x i8>* nocapture %xOut, <8 x i8>* nocapture %yOut, <8 x i8>* nocapture %zOut, <8 x i8>* nocapture %wOut) nounwind alwaysinline {
entry:
  %0 = load <4 x i8>* %pLoadAdd1, align 4
  %1 = bitcast <4 x i8> %0 to i32
  %2 = insertelement <4 x i32> undef, i32 %1, i32 0
  %splat.i = shufflevector <4 x i32> %2, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = load <4 x i8>* %pLoadAdd2, align 4
  %4 = bitcast <4 x i8> %3 to i32
  %5 = insertelement <4 x i32> undef, i32 %4, i32 0
  %splat1.i = shufflevector <4 x i32> %5, <4 x i32> undef, <4 x i32> zeroinitializer
  %6 = load <4 x i8>* %pLoadAdd3, align 4
  %7 = bitcast <4 x i8> %6 to i32
  %8 = insertelement <4 x i32> undef, i32 %7, i32 0
  %splat2.i = shufflevector <4 x i32> %8, <4 x i32> undef, <4 x i32> zeroinitializer
  %9 = load <4 x i8>* %pLoadAdd5, align 4
  %10 = bitcast <4 x i8> %9 to i32
  %11 = insertelement <4 x i32> undef, i32 %10, i32 0
  %splat3.i = shufflevector <4 x i32> %11, <4 x i32> undef, <4 x i32> zeroinitializer
  %12 = load <4 x i8>* %pLoadAdd6, align 4
  %13 = bitcast <4 x i8> %12 to i32
  %14 = insertelement <4 x i32> undef, i32 %13, i32 0
  %splat4.i = shufflevector <4 x i32> %14, <4 x i32> undef, <4 x i32> zeroinitializer
  %15 = load <4 x i8>* %pLoadAdd7, align 4
  %16 = bitcast <4 x i8> %15 to i32
  %17 = insertelement <4 x i32> undef, i32 %16, i32 0
  %splat5.i = shufflevector <4 x i32> %17, <4 x i32> undef, <4 x i32> zeroinitializer
  %18 = load <4 x i8>* %pLoadAdd0, align 4
  %19 = bitcast <4 x i8> %18 to i32
  %20 = insertelement <4 x i32> undef, i32 %19, i32 0
  %21 = bitcast <4 x i32> %20 to <4 x float>
  %22 = bitcast <4 x i32> %splat.i to <4 x float>
  %23 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %21, <4 x float> %22, i32 2) nounwind
  %24 = bitcast <4 x i32> %splat1.i to <4 x float>
  %25 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %23, <4 x float> %24, i32 4) nounwind
  %26 = bitcast <4 x i32> %splat2.i to <4 x float>
  %27 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %25, <4 x float> %26, i32 8) nounwind
  %28 = load <4 x i8>* %pLoadAdd4, align 4
  %29 = bitcast <4 x i8> %28 to i32
  %30 = insertelement <4 x i32> undef, i32 %29, i32 0
  %31 = bitcast <4 x i32> %30 to <4 x float>
  %32 = bitcast <4 x i32> %splat3.i to <4 x float>
  %33 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %31, <4 x float> %32, i32 2) nounwind
  %34 = bitcast <4 x i32> %splat4.i to <4 x float>
  %35 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %33, <4 x float> %34, i32 4) nounwind
  %36 = bitcast <4 x i32> %splat5.i to <4 x float>
  %37 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %35, <4 x float> %36, i32 8) nounwind
  %astype21.i = bitcast <4 x float> %27 to <16 x i8>
  %astype22.i = bitcast <4 x float> %37 to <16 x i8>
  %38 = shufflevector <16 x i8> %astype21.i, <16 x i8> undef, <16 x i32> <i32 0, i32 4, i32 8, i32 12, i32 1, i32 5, i32 9, i32 13, i32 2, i32 6, i32 10, i32 14, i32 3, i32 7, i32 11, i32 15>
  %39 = shufflevector <16 x i8> %astype22.i, <16 x i8> undef, <16 x i32> <i32 0, i32 4, i32 8, i32 12, i32 1, i32 5, i32 9, i32 13, i32 2, i32 6, i32 10, i32 14, i32 3, i32 7, i32 11, i32 15>
  %astype.i.i = bitcast <16 x i8> %38 to <4 x i32>
  %astype1.i.i = bitcast <16 x i8> %39 to <4 x i32>
  %call.i.i = tail call <4 x i32> @_Z8shuffle2Dv4_iS_Dv4_j(<4 x i32> %astype.i.i, <4 x i32> %astype1.i.i, <4 x i32> <i32 0, i32 4, i32 1, i32 5>) nounwind readnone
  %astype2.i.i = bitcast <4 x i32> %call.i.i to <16 x i8>
  %call5.i.i = tail call <4 x i32> @_Z8shuffle2Dv4_iS_Dv4_j(<4 x i32> %astype.i.i, <4 x i32> %astype1.i.i, <4 x i32> <i32 2, i32 6, i32 3, i32 7>) nounwind readnone
  %astype6.i.i = bitcast <4 x i32> %call5.i.i to <16 x i8>
  %40 = shufflevector <16 x i8> %astype2.i.i, <16 x i8> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  store <8 x i8> %40, <8 x i8>* %xOut, align 8
  %41 = shufflevector <16 x i8> %astype2.i.i, <16 x i8> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  store <8 x i8> %41, <8 x i8>* %yOut, align 8
  %42 = shufflevector <16 x i8> %astype6.i.i, <16 x i8> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  store <8 x i8> %42, <8 x i8>* %zOut, align 8
  %43 = shufflevector <16 x i8> %astype6.i.i, <16 x i8> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  store <8 x i8> %43, <8 x i8>* %wOut, align 8
  ret void
}

define void @__ocl_gather_transpose_char4x4(<4 x i8>* nocapture %pLoadAdd0, <4 x i8>* nocapture %pLoadAdd1, <4 x i8>* nocapture %pLoadAdd2, <4 x i8>* nocapture %pLoadAdd3, <4 x i8>* nocapture %xOut, <4 x i8>* nocapture %yOut, <4 x i8>* nocapture %zOut, <4 x i8>* nocapture %wOut) nounwind alwaysinline {
entry:
  %0 = load <4 x i8>* %pLoadAdd1, align 4
  %1 = bitcast <4 x i8> %0 to i32
  %2 = insertelement <4 x i32> undef, i32 %1, i32 0
  %splat = shufflevector <4 x i32> %2, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = load <4 x i8>* %pLoadAdd2, align 4
  %4 = bitcast <4 x i8> %3 to i32
  %5 = insertelement <4 x i32> undef, i32 %4, i32 0
  %splat1 = shufflevector <4 x i32> %5, <4 x i32> undef, <4 x i32> zeroinitializer
  %6 = load <4 x i8>* %pLoadAdd3, align 4
  %7 = bitcast <4 x i8> %6 to i32
  %8 = insertelement <4 x i32> undef, i32 %7, i32 0
  %splat2 = shufflevector <4 x i32> %8, <4 x i32> undef, <4 x i32> zeroinitializer
  %9 = load <4 x i8>* %pLoadAdd0, align 4
  %10 = bitcast <4 x i8> %9 to i32
  %11 = insertelement <4 x i32> undef, i32 %10, i32 0
  %12 = bitcast <4 x i32> %11 to <4 x float>
  %13 = bitcast <4 x i32> %splat to <4 x float>
  %14 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %12, <4 x float> %13, i32 2)
  %15 = bitcast <4 x i32> %splat1 to <4 x float>
  %16 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %14, <4 x float> %15, i32 4)
  %17 = bitcast <4 x i32> %splat2 to <4 x float>
  %18 = tail call <4 x float> @llvm.x86.sse41.blendps(<4 x float> %16, <4 x float> %17, i32 8)
  %astype9 = bitcast <4 x float> %18 to <16 x i8>
  %19 = shufflevector <16 x i8> %astype9, <16 x i8> undef, <4 x i32> <i32 0, i32 4, i32 8, i32 12>
  store <4 x i8> %19, <4 x i8>* %xOut, align 4
  %20 = bitcast <4 x float> %18 to <2 x i64>
  %21 = tail call <2 x i64> @llvm.x86.sse2.psrl.dq(<2 x i64> %20, i32 8) nounwind
  %astype.i = bitcast <2 x i64> %21 to <16 x i8>
  %22 = shufflevector <16 x i8> %astype.i, <16 x i8> undef, <4 x i32> <i32 0, i32 4, i32 8, i32 12>
  store <4 x i8> %22, <4 x i8>* %yOut, align 4
  %23 = tail call <2 x i64> @llvm.x86.sse2.psrl.dq(<2 x i64> %21, i32 8) nounwind
  %astype2.i = bitcast <2 x i64> %23 to <16 x i8>
  %24 = shufflevector <16 x i8> %astype2.i, <16 x i8> undef, <4 x i32> <i32 0, i32 4, i32 8, i32 12>
  store <4 x i8> %24, <4 x i8>* %zOut, align 4
  %25 = tail call <2 x i64> @llvm.x86.sse2.psrl.dq(<2 x i64> %23, i32 8) nounwind
  %astype4.i = bitcast <2 x i64> %25 to <16 x i8>
  %26 = shufflevector <16 x i8> %astype4.i, <16 x i8> undef, <4 x i32> <i32 0, i32 4, i32 8, i32 12>
  store <4 x i8> %26, <4 x i8>* %wOut, align 4
  ret void
}
