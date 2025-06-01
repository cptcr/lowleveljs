#include "headers/lljs.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <vector>
#include <complex>
#include <numeric>
#include <immintrin.h> // For SIMD operations
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace LLJS::Math {

// Global random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

/**
 * Fast square root implementation using hardware acceleration
 * @param info - CallbackInfo containing number parameter
 * @returns Square root of input
 */
Napi::Value FastSqrt(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number parameter required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    double x = info[0].As<Napi::Number>().DoubleValue();
    
    if (x < 0) {
        return Napi::Number::New(env, std::numeric_limits<double>::quiet_NaN());
    }
    
    // Use hardware square root instruction for maximum performance
    double result = std::sqrt(x);
    return Napi::Number::New(env, result);
}

/**
 * Fast inverse square root (Quake III algorithm)
 * @param info - CallbackInfo containing number parameter
 * @returns 1/sqrt(x)
 */
Napi::Value FastInvSqrt(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number parameter required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    float x = info[0].As<Napi::Number>().FloatValue();
    
    if (x <= 0) {
        return Napi::Number::New(env, std::numeric_limits<double>::infinity());
    }
    
    // Fast inverse square root algorithm
    float x2 = x * 0.5f;
    float y = x;
    uint32_t i = *(uint32_t*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (x2 * y * y)); // Newton iteration
    y = y * (1.5f - (x2 * y * y)); // Second iteration for better precision
    
    return Napi::Number::New(env, y);
}

/**
 * SIMD-accelerated vector operations
 * @param info - CallbackInfo containing operation configuration
 * @returns Operation result
 */
Napi::Value VectorOperations(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Operation object required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object operation = info[0].As<Napi::Object>();
    std::string op = operation.Get("operation").As<Napi::String>();
    Napi::Array vectorA = operation.Get("a").As<Napi::Array>();
    
    std::vector<double> a;
    for (uint32_t i = 0; i < vectorA.Length(); i++) {
        a.push_back(vectorA.Get(i).As<Napi::Number>().DoubleValue());
    }
    
    if (op == "add" || op == "subtract" || op == "multiply" || op == "divide") {
        if (!operation.Has("b")) {
            Napi::TypeError::New(env, "Vector b required for binary operations").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Array vectorB = operation.Get("b").As<Napi::Array>();
        std::vector<double> b;
        for (uint32_t i = 0; i < vectorB.Length(); i++) {
            b.push_back(vectorB.Get(i).As<Napi::Number>().DoubleValue());
        }
        
        size_t minSize = std::min(a.size(), b.size());
        Napi::Array result = Napi::Array::New(env);
        
        // Use SIMD for vectorized operations when possible
        size_t simdSize = (minSize / 4) * 4; // Process in chunks of 4
        
        for (size_t i = 0; i < simdSize; i += 4) {
            __m256d vecA = _mm256_loadu_pd(&a[i]);
            __m256d vecB = _mm256_loadu_pd(&b[i]);
            __m256d vecResult;
            
            if (op == "add") {
                vecResult = _mm256_add_pd(vecA, vecB);
            } else if (op == "subtract") {
                vecResult = _mm256_sub_pd(vecA, vecB);
            } else if (op == "multiply") {
                vecResult = _mm256_mul_pd(vecA, vecB);
            } else if (op == "divide") {
                vecResult = _mm256_div_pd(vecA, vecB);
            }
            
            double resultArray[4];
            _mm256_storeu_pd(resultArray, vecResult);
            
            for (int j = 0; j < 4; j++) {
                result.Set(i + j, Napi::Number::New(env, resultArray[j]));
            }
        }
        
        // Handle remaining elements
        for (size_t i = simdSize; i < minSize; i++) {
            double value = 0;
            if (op == "add") value = a[i] + b[i];
            else if (op == "subtract") value = a[i] - b[i];
            else if (op == "multiply") value = a[i] * b[i];
            else if (op == "divide") value = a[i] / b[i];
            result.Set(i, Napi::Number::New(env, value));
        }
        
        return result;
    } else if (op == "dot") {
        if (!operation.Has("b")) {
            Napi::TypeError::New(env, "Vector b required for dot product").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Array vectorB = operation.Get("b").As<Napi::Array>();
        std::vector<double> b;
        for (uint32_t i = 0; i < vectorB.Length(); i++) {
            b.push_back(vectorB.Get(i).As<Napi::Number>().DoubleValue());
        }
        
        double result = 0;
        size_t minSize = std::min(a.size(), b.size());
        
        // SIMD dot product
        size_t simdSize = (minSize / 4) * 4;
        __m256d sum = _mm256_setzero_pd();
        
        for (size_t i = 0; i < simdSize; i += 4) {
            __m256d vecA = _mm256_loadu_pd(&a[i]);
            __m256d vecB = _mm256_loadu_pd(&b[i]);
            __m256d product = _mm256_mul_pd(vecA, vecB);
            sum = _mm256_add_pd(sum, product);
        }
        
        // Extract and sum the 4 doubles
        double sumArray[4];
        _mm256_storeu_pd(sumArray, sum);
        result = sumArray[0] + sumArray[1] + sumArray[2] + sumArray[3];
        
        // Handle remaining elements
        for (size_t i = simdSize; i < minSize; i++) {
            result += a[i] * b[i];
        }
        
        return Napi::Number::New(env, result);
    } else if (op == "cross") {
        if (!operation.Has("b") || a.size() != 3 || vectorB.Length() != 3) {
            Napi::TypeError::New(env, "Cross product requires two 3D vectors").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Array vectorB = operation.Get("b").As<Napi::Array>();
        std::vector<double> b = {
            vectorB.Get(0).As<Napi::Number>().DoubleValue(),
            vectorB.Get(1).As<Napi::Number>().DoubleValue(),
            vectorB.Get(2).As<Napi::Number>().DoubleValue()
        };
        
        Napi::Array result = Napi::Array::New(env);
        result.Set(0, Napi::Number::New(env, a[1] * b[2] - a[2] * b[1]));
        result.Set(1, Napi::Number::New(env, a[2] * b[0] - a[0] * b[2]));
        result.Set(2, Napi::Number::New(env, a[0] * b[1] - a[1] * b[0]));
        
        return result;
    } else if (op == "magnitude") {
        double sum = 0;
        for (double val : a) {
            sum += val * val;
        }
        return Napi::Number::New(env, std::sqrt(sum));
    } else if (op == "normalize") {
        double magnitude = 0;
        for (double val : a) {
            magnitude += val * val;
        }
        magnitude = std::sqrt(magnitude);
        
        if (magnitude == 0) {
            Napi::Error::New(env, "Cannot normalize zero vector").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Array result = Napi::Array::New(env);
        for (size_t i = 0; i < a.size(); i++) {
            result.Set(i, Napi::Number::New(env, a[i] / magnitude));
        }
        
        return result;
    }
    
    Napi::TypeError::New(env, "Unknown vector operation").ThrowAsJavaScriptException();
    return env.Null();
}

/**
 * High-performance matrix operations
 * @param info - CallbackInfo containing operation configuration
 * @returns Operation result
 */
Napi::Value MatrixOperations(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Operation object required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object operation = info[0].As<Napi::Object>();
    std::string op = operation.Get("operation").As<Napi::String>();
    Napi::Array matrixArray = operation.Get("matrix").As<Napi::Array>();
    
    // Convert JavaScript matrix to C++ vector
    std::vector<std::vector<double>> matrix;
    for (uint32_t i = 0; i < matrixArray.Length(); i++) {
        Napi::Array row = matrixArray.Get(i).As<Napi::Array>();
        std::vector<double> matrixRow;
        for (uint32_t j = 0; j < row.Length(); j++) {
            matrixRow.push_back(row.Get(j).As<Napi::Number>().DoubleValue());
        }
        matrix.push_back(matrixRow);
    }
    
    if (op == "transpose") {
        size_t rows = matrix.size();
        size_t cols = rows > 0 ? matrix[0].size() : 0;
        
        Napi::Array result = Napi::Array::New(env);
        for (size_t j = 0; j < cols; j++) {
            Napi::Array resultRow = Napi::Array::New(env);
            for (size_t i = 0; i < rows; i++) {
                resultRow.Set(i, Napi::Number::New(env, matrix[i][j]));
            }
            result.Set(j, resultRow);
        }
        
        return result;
    } else if (op == "multiply") {
        if (!operation.Has("matrix2")) {
            Napi::TypeError::New(env, "Second matrix required for multiplication").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Array matrix2Array = operation.Get("matrix2").As<Napi::Array>();
        std::vector<std::vector<double>> matrix2;
        for (uint32_t i = 0; i < matrix2Array.Length(); i++) {
            Napi::Array row = matrix2Array.Get(i).As<Napi::Array>();
            std::vector<double> matrixRow;
            for (uint32_t j = 0; j < row.Length(); j++) {
                matrixRow.push_back(row.Get(j).As<Napi::Number>().DoubleValue());
            }
            matrix2.push_back(matrixRow);
        }
        
        size_t rows1 = matrix.size();
        size_t cols1 = rows1 > 0 ? matrix[0].size() : 0;
        size_t rows2 = matrix2.size();
        size_t cols2 = rows2 > 0 ? matrix2[0].size() : 0;
        
        if (cols1 != rows2) {
            Napi::TypeError::New(env, "Matrix dimensions incompatible for multiplication").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Array result = Napi::Array::New(env);
        for (size_t i = 0; i < rows1; i++) {
            Napi::Array resultRow = Napi::Array::New(env);
            for (size_t j = 0; j < cols2; j++) {
                double sum = 0;
                for (size_t k = 0; k < cols1; k++) {
                    sum += matrix[i][k] * matrix2[k][j];
                }
                resultRow.Set(j, Napi::Number::New(env, sum));
            }
            result.Set(i, resultRow);
        }
        
        return result;
    } else if (op == "determinant") {
        size_t n = matrix.size();
        if (n == 0 || matrix[0].size() != n) {
            Napi::TypeError::New(env, "Determinant requires square matrix").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        // Gaussian elimination for determinant calculation
        std::vector<std::vector<double>> temp = matrix;
        double det = 1.0;
        
        for (size_t i = 0; i < n; i++) {
            // Find pivot
            size_t maxRow = i;
            for (size_t k = i + 1; k < n; k++) {
                if (std::abs(temp[k][i]) > std::abs(temp[maxRow][i])) {
                    maxRow = k;
                }
            }
            
            if (maxRow != i) {
                std::swap(temp[i], temp[maxRow]);
                det *= -1;
            }
            
            if (std::abs(temp[i][i]) < 1e-10) {
                return Napi::Number::New(env, 0.0);
            }
            
            det *= temp[i][i];
            
            for (size_t k = i + 1; k < n; k++) {
                double factor = temp[k][i] / temp[i][i];
                for (size_t j = i; j < n; j++) {
                    temp[k][j] -= factor * temp[i][j];
                }
            }
        }
        
        return Napi::Number::New(env, det);
    } else if (op == "inverse") {
        size_t n = matrix.size();
        if (n == 0 || matrix[0].size() != n) {
            Napi::TypeError::New(env, "Inverse requires square matrix").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        // Gauss-Jordan elimination for matrix inversion
        std::vector<std::vector<double>> augmented(n, std::vector<double>(2 * n, 0));
        
        // Create augmented matrix [A|I]
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                augmented[i][j] = matrix[i][j];
            }
            augmented[i][i + n] = 1.0;
        }
        
        // Forward elimination
        for (size_t i = 0; i < n; i++) {
            // Find pivot
            size_t maxRow = i;
            for (size_t k = i + 1; k < n; k++) {
                if (std::abs(augmented[k][i]) > std::abs(augmented[maxRow][i])) {
                    maxRow = k;
                }
            }
            
            std::swap(augmented[i], augmented[maxRow]);
            
            if (std::abs(augmented[i][i]) < 1e-10) {
                Napi::Error::New(env, "Matrix is singular").ThrowAsJavaScriptException();
                return env.Null();
            }
            
            // Scale pivot row
            double pivot = augmented[i][i];
            for (size_t j = 0; j < 2 * n; j++) {
                augmented[i][j] /= pivot;
            }
            
            // Eliminate column
            for (size_t k = 0; k < n; k++) {
                if (k != i) {
                    double factor = augmented[k][i];
                    for (size_t j = 0; j < 2 * n; j++) {
                        augmented[k][j] -= factor * augmented[i][j];
                    }
                }
            }
        }
        
        // Extract inverse matrix
        Napi::Array result = Napi::Array::New(env);
        for (size_t i = 0; i < n; i++) {
            Napi::Array resultRow = Napi::Array::New(env);
            for (size_t j = 0; j < n; j++) {
                resultRow.Set(j, Napi::Number::New(env, augmented[i][j + n]));
            }
            result.Set(i, resultRow);
        }
        
        return result;
    }
    
    Napi::TypeError::New(env, "Unknown matrix operation").ThrowAsJavaScriptException();
    return env.Null();
}

/**
 * Optimized bitwise operations
 * @param info - CallbackInfo containing operation, operands
 * @returns Operation result
 */
Napi::Value BitwiseOperations(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Operation and first operand required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string operation = info[0].As<Napi::String>();
    uint64_t a = info[1].As<Napi::Number>().Uint32Value();
    uint64_t b = 0;
    
    if (info.Length() > 2 && info[2].IsNumber()) {
        b = info[2].As<Napi::Number>().Uint32Value();
    }
    
    uint64_t result = 0;
    
    if (operation == "and") {
        result = a & b;
    } else if (operation == "or") {
        result = a | b;
    } else if (operation == "xor") {
        result = a ^ b;
    } else if (operation == "not") {
        result = ~a;
    } else if (operation == "shl") {
        result = a << b;
    } else if (operation == "shr") {
        result = a >> b;
    } else if (operation == "rotl") {
        result = (a << b) | (a >> (32 - b));
    } else if (operation == "rotr") {
        result = (a >> b) | (a << (32 - b));
    } else if (operation == "popcount") {
        result = __builtin_popcountll(a);
    } else if (operation == "clz") {
        result = a == 0 ? 64 : __builtin_clzll(a);
    } else if (operation == "ctz") {
        result = a == 0 ? 64 : __builtin_ctzll(a);
    } else {
        Napi::TypeError::New(env, "Unknown bitwise operation").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    return Napi::Number::New(env, static_cast<double>(result));
}

/**
 * Advanced random number generation with various distributions
 * @param info - CallbackInfo containing count, min, max, distribution
 * @returns Array of random numbers
 */
Napi::Value RandomNumbers(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Count parameter required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    int count = info[0].As<Napi::Number>().Int32Value();
    double min = 0.0;
    double max = 1.0;
    std::string distribution = "uniform";
    
    if (info.Length() > 1 && info[1].IsNumber()) {
        min = info[1].As<Napi::Number>().DoubleValue();
    }
    
    if (info.Length() > 2 && info[2].IsNumber()) {
        max = info[2].As<Napi::Number>().DoubleValue();
    }
    
    if (info.Length() > 3 && info[3].IsString()) {
        distribution = info[3].As<Napi::String>();
    }
    
    if (count <= 0) {
        Napi::TypeError::New(env, "Count must be positive").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Array result = Napi::Array::New(env);
    
    if (distribution == "uniform") {
        std::uniform_real_distribution<double> dist(min, max);
        for (int i = 0; i < count; i++) {
            result.Set(i, Napi::Number::New(env, dist(gen)));
        }
    } else if (distribution == "normal") {
        // min = mean, max = stddev
        std::normal_distribution<double> dist(min, max);
        for (int i = 0; i < count; i++) {
            result.Set(i, Napi::Number::New(env, dist(gen)));
        }
    } else if (distribution == "exponential") {
        std::exponential_distribution<double> dist(min); // min = lambda
        for (int i = 0; i < count; i++) {
            result.Set(i, Napi::Number::New(env, dist(gen)));
        }
    } else if (distribution == "gamma") {
        std::gamma_distribution<double> dist(min, max); // min = alpha, max = beta
        for (int i = 0; i < count; i++) {
            result.Set(i, Napi::Number::New(env, dist(gen)));
        }
    } else if (distribution == "poisson") {
        std::poisson_distribution<int> dist(min); // min = mean
        for (int i = 0; i < count; i++) {
            result.Set(i, Napi::Number::New(env, dist(gen)));
        }
    } else {
        Napi::TypeError::New(env, "Unknown distribution type").ThrowAsJavaScriptException();
        return env.Null();
    }
    

}

/**
 * Fast Fourier Transform implementation
 * @param info - CallbackInfo containing complex number array
 * @returns FFT result
 */
Napi::Value FastFourierTransform(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "Complex number array required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Array inputArray = info[0].As<Napi::Array>();
    std::vector<std::complex<double>> data;
    
    for (uint32_t i = 0; i < inputArray.Length(); i++) {
        if (inputArray.Get(i).IsArray()) {
            Napi::Array complexNum = inputArray.Get(i).As<Napi::Array>();
            double real = complexNum.Get(0).As<Napi::Number>().DoubleValue();
            double imag = complexNum.Length() > 1 ? complexNum.Get(1).As<Napi::Number>().DoubleValue() : 0.0;
            data.emplace_back(real, imag);
        } else {
            double real = inputArray.Get(i).As<Napi::Number>().DoubleValue();
            data.emplace_back(real, 0.0);
        }
    }
    
    // Ensure data size is power of 2
    size_t n = data.size();
    size_t powerOf2 = 1;
    while (powerOf2 < n) powerOf2 <<= 1;
    data.resize(powerOf2, std::complex<double>(0, 0));
    
    // Cooley-Tukey FFT algorithm
    std::function<void(std::vector<std::complex<double>>&)> fft = 
        [&](std::vector<std::complex<double>>& x) {
            size_t N = x.size();
            if (N <= 1) return;
            
            // Divide
            std::vector<std::complex<double>> even, odd;
            for (size_t i = 0; i < N; i += 2) {
                even.push_back(x[i]);
                if (i + 1 < N) odd.push_back(x[i + 1]);
            }
            
            // Conquer
            fft(even);
            fft(odd);
            
            // Combine
            for (size_t k = 0; k < N / 2; k++) {
                std::complex<double> t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
                x[k] = even[k] + t;
                x[k + N / 2] = even[k] - t;
            }
        };
    
    fft(data);
    
    // Convert result back to JavaScript array
    Napi::Array result = Napi::Array::New(env);
    for (size_t i = 0; i < data.size(); i++) {
        Napi::Array complexNum = Napi::Array::New(env);
        complexNum.Set(0U, Napi::Number::New(env, data[i].real()));
        complexNum.Set(1U, Napi::Number::New(env, data[i].imag()));
        result.Set(i, complexNum);
    }
    
    return result;
}