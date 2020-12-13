#include <sstream>
#include <ostream>
#include <ctime>

#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

#include "Block.h"

namespace nl = nlohmann;

namespace ash
{

void to_json(nl::json& j, const Block& b)
{
    j["index"] = b.index();
    j["nonce"] = b.nonce();
    j["difficulty"] = b.difficulty();
    j["data"] = b.data();
    j["time"] = b.time();
    j["hash"] = b.hash();
    j["prev"] = b.previousHash();
    j["miner"] = b.miner();
}

void from_json(const nl::json& j, Block& b)
{
    j["index"].get_to(b._hashed._index);
    j["nonce"].get_to(b._hashed._nonce);
    j["difficulty"].get_to(b._hashed._difficulty);
    j["data"].get_to(b._hashed._data);
    j["time"].get_to(b._hashed._time);
    j["prev"].get_to(b._hashed._prev);
    j["hash"].get_to(b._hash);
    j["miner"].get_to(b._miner);
}

std::string SHA256(std::string data)
{
    std::string digest;
    CryptoPP::SHA256 hash;

    CryptoPP::StringSource foo(data, true,
        new CryptoPP::HashFilter(hash,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(digest), false)));

    return digest;
}

std::string CalculateBlockHash(
    std::uint64_t index, 
    std::uint32_t nonce, 
    std::uint32_t difficulty,
    time_t time, 
    const std::string& data, 
    const std::string& previous)
{
    std::stringstream ss;
    ss << index
        << nonce
        << difficulty
        << data
        << time
        << previous;

    return SHA256(ss.str());
}

std::string CalculateBlockHash(const Block& block)
{
    std::stringstream ss;
    ss << block.index()
        << block.nonce()
        << block.difficulty()
        << block.data()
        << block.time()
        << block.previousHash();

    return SHA256(ss.str());
}

Block::Block(uint64_t nIndexIn, std::string_view sDataIn)
    : _logger(ash::initializeLogger("Block"))
{
    _hashed._index = nIndexIn;
    _hashed._data = sDataIn;
    _hashed._nonce = 0;
    _hashed._time = 0;
    // _hash = CalculateBlockHash(*this);
}

bool Block::operator==(const Block & other) const
{
    return _hashed._index == other._hashed._index
        && _hashed._nonce == other._hashed._nonce
        && _hashed._data == other._hashed._data
        && _hashed._time == other._hashed._time;
}

} // namespace

namespace std
{

std::ostream& operator<<(std::ostream & os, const ash::Block & block)
{
    os << "block { index: " << block.index() << " hash: " << block.hash() << " data: " << block.data() << " }";
    return os;
}

} // namespace std