void transformer1_v1(float* inputA, float* inputB, float* outputC, int size) {
    for (int i = 0; i < size; ++i) {
        outputC[i] = inputA[i] * inputB[i];
    }
}