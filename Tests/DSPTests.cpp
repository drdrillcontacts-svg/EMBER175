#include "EmberDSP.h"
#include <cmath>
#include <iostream>
#include <vector>

static bool finiteOutput(const std::vector<float>& x)
{
    for (float v : x) if (!std::isfinite(v)) return false;
    return true;
}

int main()
{
    constexpr int n = 48000;
    ember::Processor proc;
    proc.prepare(48000.0, 512, 2);

    // Test 1: bypass must preserve the input exactly.
    std::vector<float> l(n), r(n);
    for (int i = 0; i < n; ++i) {
        l[i] = 0.2f * std::sin(2.0 * 3.141592653589793 * 440.0 * i / 48000.0);
        r[i] = 0.1f * std::sin(2.0 * 3.141592653589793 * 330.0 * i / 48000.0);
    }
    auto originalL = l, originalR = r;
    float* ptrs[2] { l.data(), r.data() };
    ember::Parameters p;
    p.bypass = true;
    proc.process(ptrs, 2, n, p);
    for (int i = 0; i < n; ++i) {
        if (l[i] != originalL[i] || r[i] != originalR[i]) {
            std::cerr << "FAIL: bypass is not transparent\n";
            return 1;
        }
    }

    // Test 2: active DSP should produce finite output.
    p.bypass = false;
    p.inputDb = 8.0f;
    p.thresholdDb = -24.0f;
    p.color = 0.3f;
    proc.reset();
    proc.process(ptrs, 2, n, p);
    if (!finiteOutput(l) || !finiteOutput(r)) {
        std::cerr << "FAIL: non-finite output\n";
        return 2;
    }

    // Test 3: gain reduction should be non-positive and finite.
    const float gr = proc.gainReductionDb();
    if (!std::isfinite(gr) || gr > 0.001f) {
        std::cerr << "FAIL: invalid gain reduction\n";
        return 3;
    }

    std::cout << "PASS: bypass, finite-output, and gain-reduction sanity tests\n";
    return 0;
}
