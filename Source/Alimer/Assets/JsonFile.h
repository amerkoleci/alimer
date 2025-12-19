// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Assets/Asset.h"
#include "Alimer/Assets/JsonValue.h"

namespace Alimer
{
    /// JSON document. Contains a root JSON value and can be read/written to file as text.
    class ALIMER_API JsonFile final : public Asset
    {
        ALIMER_OBJECT(JsonFile, Asset);

    public:
        static void Register();

        /// Constructor.
        explicit JsonFile(std::string_view name = kEmptyStringView);

        /// Deserialize from a string. Return true if successful.
        bool FromString(const std::string& source);
        /// Save to a string.
        std::string ToString(const std::string& indendation = "\t") const;

        /// Load from a stream as text. Return true on success. Will contain partial data on failure.
        bool BeginLoad(Stream& source) override;

        /// Save to a stream as text. Return true on success.
        bool Save(Stream& stream) const override;

        /// Return root value.
        JsonValue& GetRoot() { return root; }
        /// Return root value.
        const JsonValue& GetRoot() const { return root; }

    private:
        /// Root value.
        JsonValue root;
    };
}
