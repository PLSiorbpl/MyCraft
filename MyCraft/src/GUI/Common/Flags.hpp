#pragma once

/*
    Utility for manipulating packed 32-bit GUI flags

    Flags are stored as a single uint32_t and passed directly
    to the GPU as a compact per-vertex / per-instance value

Responsibilities:
    - Set, clear and query individual bit flags
    - Pack and unpack small integer values into flag bit ranges
      (e.g. TextureId)

Design:
    - Bit layout is part of the GUI <-> shader contract
    - Lower-level utility with no ownership or state
    - Intended to be cheap, inline and branchless

Notes:
    - TextureId occupies a fixed bit range defined by TextureIdBits
    - Flags are interpreted in shaders, not on CPU side
*/

constexpr uint32_t TextureIdBits = 4u;
constexpr uint32_t TextureIdMask = (1u << TextureIdBits) - 1u; // 0xF

class Flags32 {
public:
    static inline void Set(uint32_t &flags, const int bit) {flags |= (1u << bit);}
    static inline void Clear(uint32_t &flags, const int bit) {flags &= ~(1u << bit);}
    static inline void Toggle(uint32_t &flags, const int bit) {flags ^= (1u << bit);}
    static inline bool Get(const uint32_t &flags, const int bit) {return flags & (1u << bit);}
    static inline void SetTextureId(uint32_t& flags, int id) {
        id &= TextureIdMask;
        flags &= ~(TextureIdMask << static_cast<uint32_t>(gui::FlagBit::TextureId));
        flags |= (id << static_cast<uint32_t>(gui::FlagBit::TextureId));
    }
    static inline int GetTextureId(uint32_t flags) {
        return (flags >> static_cast<uint32_t>(gui::FlagBit::TextureId)) & TextureIdMask;
    }
};