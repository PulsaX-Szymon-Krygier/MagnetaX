// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Check.h>
#include <limits>
#include <vector>

template<typename Key>
class SparseSet
{
public:
    usize Insert(Key key)
    {
        MX_CHECK(!Has(key), "Cannot insert duplicate key into SparseSet");

        const usize sparseIndex = static_cast<usize>(key);

        if (sparseIndex >= sparse.size()) sparse.resize(sparseIndex + 1, INVALID_INDEX);

        const usize denseIndex = dense.size();

        dense.push_back(key);
        sparse[sparseIndex] = denseIndex;

        return denseIndex;
    }

    void Remove(Key key)
    {
        if (!Has(key)) return;

        const usize sparseIndex = static_cast<usize>(key);
        const usize denseIndex = sparse[sparseIndex];
        const usize lastDenseIndex = dense.size() - 1;

        if (denseIndex != lastDenseIndex)
        {
            dense[denseIndex] = dense[lastDenseIndex];
            sparse[static_cast<usize>(dense[denseIndex])] = denseIndex;
        }

        dense.pop_back();
        sparse[sparseIndex] = INVALID_INDEX;
    }

    bool Has(Key key) const
    {
        const usize sparseIndex = static_cast<usize>(key);
        return sparseIndex < sparse.size() && sparse[sparseIndex] != INVALID_INDEX;
    }

    usize IndexOf(Key key) const
    {
        return Has(key) ? sparse[static_cast<usize>(key)] : INVALID_INDEX;
    }

    usize Size() const { return dense.size(); }

    Key KeyAt(usize index) const { return dense[index]; }

private:
    static constexpr usize INVALID_INDEX = std::numeric_limits<usize>::max();

    std::vector<usize> sparse;
    std::vector<Key> dense;
};
