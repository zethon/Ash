#pragma once
#include "Block.h"
#include "AshLogger.h"

namespace ash
{

class Miner
{
    std::uint32_t       _difficulty = 0;
    std::uint64_t       _maxTries = 0;
    std::atomic_bool    _keepTrying = true;
    std::uint32_t       _timeout; // seconds
    SpdLogPtr           _logger;

public:
    enum ResultType { SUCCESS, TIMEOUT, ABORT };
    using Result = std::tuple<ResultType, Block>;

    Miner()
        : Miner(0)
    {
        // nothing to do
    }

    Miner(std::uint32_t difficulty)
        : _difficulty{difficulty},
          _logger(ash::initializeLogger("Miner"))
    {
        // nothing to do
    }

    std::uint32_t difficulty() const noexcept { return _difficulty; }
    void setDifficulty(std::uint32_t val) { _difficulty = val; }

    void abort() 
    { 
        _keepTrying.store(false, std::memory_order_release);
    }

    Result mineBlock(std::uint64_t index, 
        const std::string& data, 
        const std::string& prev,
        std::function<bool(std::uint64_t)> keepGoingFunc = nullptr)
    {
        assert(index > 0);
        assert(prev.size() > 0 || (index - 1 == 0));

        std::string zeros;
        zeros.assign(_difficulty, '0');

        std::uint32_t nonce = 0;
        auto time = 
            std::chrono::time_point_cast<std::chrono::milliseconds>
                (std::chrono::system_clock::now());

        std::string hash = 
            CalculateBlockHash(index, nonce, _difficulty, time, data, prev);

        _keepTrying = true;

        while (_keepTrying.load(std::memory_order_acquire) 
            && hash.compare(0, _difficulty, zeros) != 0)
        {
            // do some extra stuff every few seconds
            if ((nonce & 0x3ffff) == 0)
            {
                if (keepGoingFunc && !keepGoingFunc(index))
                {
                    // our callback has told us to bail
                    return { ResultType::ABORT, {} };
                }

                time = 
                    std::chrono::time_point_cast<std::chrono::milliseconds>
                        (std::chrono::system_clock::now());
            }

            nonce++;
            hash = CalculateBlockHash(index, nonce, _difficulty, time, data, prev);
        }

        if (!_keepTrying)
        {
            return { ResultType::ABORT, {} };
        }

        // TODO: this function might only need to return 'nonce', 
        // 'time' and 'hash' instead of a whole new block
        Block retval { index, data };
        retval._hashed._nonce = nonce;
        retval._hashed._difficulty = _difficulty;
        retval._hashed._time = time;
        retval._hashed._prev = prev;
        retval._hash = hash;

        nl::json j = retval;
        std::cout << "RETVAL: " << j.dump() << '\n';
        _logger->info("successfully mined bock {}", index);
        return { ResultType::SUCCESS, retval };
    }
};

} // namespace ash