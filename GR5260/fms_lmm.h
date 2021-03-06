// fms_lmm.h - LIBOR Market Model
#pragma once
#include <algorithm>
#include "fms_brownian.h"
#include "fms_pwflat.h"
/*
The LIBOR Market Model is parameterized by increasing times t_j,
futures quotes phi_j, at-the-money caplet volatilities, sigma_j, 
and a d x d correlation matrix, rho_{j,k}. 
The j-th future corresponds to the interval from t_{j-1} to t_j, j > 0
Just as for fsm::pwflat, we use the conventon t_{-1} = 0 so
phi_0 is the cd rate and sigma_0 = 0.

Let Phi_j(t) = phi_j exp(sigma_j B_j(t) - sigma_j^2 t/2) be the futures quote
at time t of the j-th futures, where B_t is d-dimensional correlated standard
Brownian motion.

To account for convexity we let F_j(t) = Phi_j(t) - sigma_j^2 (t_{j-1} - t)^2/2
be the forward at time t over the interval from t_{j-1} to t_j. In
our universal notation this is F_t(t_{j-1}, t_j).

Recall D(u) = exp(-int_0^u f(s) ds), where f(s) is the current forward curve, 
and D_t(u) = exp_t^u f_t(s) ds, where s -> f_t(s) is the forward curve at time t.
Given LMM data and a time t, we would like to generate a sample forward
curve at time t, s -> f_t(s). Note f_0(s) = f(s).

*/

namespace fms {

    template<class T = double, class F = double>
    struct lmm {
        std::vector<T> t;
        std::vector<F> phi;
        std::vector<F> sigma;
        fms::brownian<F> B;

        lmm(size_t n, const T* t, const F* phi, const F* sigma, const correlation<F>& e)
            : t(t, t + n), phi(phi, phi + n), sigma(sigma, sigma + n), B(e)
        {
        }

        size_t size() const
        {
            return t.size();
        }

        void reset()
        {
            B.reset();
        }

        // Populate f_ with sample forward curve at time u and return index of first t[j] > u
        template<class R>
        size_t advance(T u, F* f_, R& r)
        {
            auto tj = std::upper_bound(t.begin(), t.end(), u);
            // tj[-1] <= u < tj[0];
            ensure (tj != t.end());
            auto j = tj - t.begin();

            B.advance(u, r);
            for (size_t k = j; k < t.size(); ++k) {
                // futures
                f_[k] = phi[k]*exp(sigma[k]*B[k] - sigma[k]*sigma[k]*u/2);
                // forward convexity adjustment
                if (k > 0) {
                    auto dt = t[k-1] - u; // k-th future settles at t[k-1]
                    f_[k] -= sigma[k]*sigma[k]*dt*dt/2;
                }
            }

            return j; 
        }
    };
}