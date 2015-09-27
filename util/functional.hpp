/*
 * (C) Copyright 2015 ETH Zurich Systems Group (http://www.systems.ethz.ch/) and others.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 *     Markus Pilman <mpilman@inf.ethz.ch>
 *     Simon Loesing <sloesing@inf.ethz.ch>
 *     Thomas Etter <etterth@gmail.com>
 *     Kevin Bocksrocker <kevin.bocksrocker@gmail.com>
 *     Lucas Braun <braunl@inf.ethz.ch>
 */
#pragma once
#include <config.h>

#include "PageManager.hpp"

#include <cstring>
#include <random>

namespace tell {
namespace store {
class PageWrapper {
public:
    static constexpr size_t ENTRIES_PER_PAGE = TELL_PAGE_SIZE / (sizeof(uint64_t) + sizeof(void*));
    static_assert(isPowerOf2(ENTRIES_PER_PAGE), "Entries per page needs to be a power of two");
    using EntryT = std::pair<uint64_t, void*>;
private:
    PageManager& mPageManager;
    EntryT* mPage;
public:
    explicit PageWrapper(PageManager& pageManager, void* page)
        : mPageManager(pageManager), mPage(reinterpret_cast<EntryT*>(page)) {
    }

    PageWrapper(const PageWrapper& other)
        : mPageManager(other.mPageManager), mPage(reinterpret_cast<EntryT*>(mPageManager.alloc())) {
        memcpy(mPage, other.mPage, TELL_PAGE_SIZE);
    }

    ~PageWrapper() {
        mPageManager.free(mPage);
    }

    const EntryT& operator[](size_t idx) const {
        return mPage[idx];
    }

    EntryT& operator[](size_t idx) {
        return mPage[idx];
    }
};

/**
* This is a very simple general hash function.
*/
class cuckoo_hash_function {
    uint64_t mSalt;
    uint64_t w_M; // m==64 w_M = w - M
public:
    cuckoo_hash_function(size_t tableSize)
        : mSalt(0) {
        std::random_device rd;
        std::uniform_int_distribution<uint64_t> dist;
        while (mSalt == 0) {
            mSalt = dist(rd);
        }
        uint64_t M = log2Of(uint64_t(tableSize));
        w_M = 64 - M;
    }

    size_t operator()(uint64_t k) const {
        return (mSalt*k) >> (w_M);
    }
};

} // namespace store
} // namespace tell
