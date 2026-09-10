#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <utility>

namespace axtual {

enum class Availability { kUnknown, kReady, kUnavailable };

template <typename T>
class SemanticState {
public:
    void setDesired(T value) { _desired = value; }
    void setObserved(T value) { _observed = value; }
    void setEffective(T value) { _effective = value; }
    void setDerived(T value) { _derived = value; }

    [[nodiscard]] const std::optional<T>& desired() const { return _desired; }
    [[nodiscard]] const std::optional<T>& observed() const { return _observed; }
    [[nodiscard]] const std::optional<T>& effective() const { return _effective; }
    [[nodiscard]] const std::optional<T>& derived() const { return _derived; }

private:
    std::optional<T> _desired;
    std::optional<T> _observed;
    std::optional<T> _effective;
    std::optional<T> _derived;
};

class RequestLifecycle {
public:
    void accept() { _accepted = true; }
    void markApplied() { _applied = true; }
    void markEffective() { _effective = true; }

    [[nodiscard]] bool accepted() const { return _accepted; }
    [[nodiscard]] bool applied() const { return _applied; }
    [[nodiscard]] bool effective() const { return _effective; }

private:
    bool _accepted = false;
    bool _applied = false;
    bool _effective = false;
};

struct GenerationIdentity {
    std::uint64_t desiredRevision = 1;
    std::uint64_t workerGeneration = 1;
    std::uint64_t sourceConnectionGeneration = 1;

    friend bool operator==(const GenerationIdentity&, const GenerationIdentity&) = default;
};

class IdentityGate {
public:
    [[nodiscard]] GenerationIdentity currentWorkIdentity() const { return _identity; }
    [[nodiscard]] GenerationIdentity currentPayloadWatermark() const { return _identity; }

    void advanceDesiredRevision() { ++_identity.desiredRevision; }
    void restartWorker() { ++_identity.workerGeneration; }
    void reconnectSource() { ++_identity.sourceConnectionGeneration; }

    [[nodiscard]] bool acceptsCompletion(const GenerationIdentity& identity) const {
        return identity == _identity;
    }

    [[nodiscard]] bool acceptsPayload(const GenerationIdentity& identity) const {
        return identity == _identity;
    }

private:
    GenerationIdentity _identity;
};

enum class SafetyKind { kMute, kPrivacy };

class SafetyStore {
public:
    virtual ~SafetyStore() = default;
    virtual bool read(bool& mute, bool& privacy) = 0;
    virtual bool write(bool mute, bool privacy) = 0;
};

class MemorySafetyStore final : public SafetyStore {
public:
    bool read(bool& mute, bool& privacy) override {
        if (_corrupt || !_value.has_value()) {
            return false;
        }
        mute = _value->first;
        privacy = _value->second;
        return true;
    }

    bool write(bool mute, bool privacy) override {
        _value = std::pair{mute, privacy};
        return true;
    }

    void corrupt() { _corrupt = true; }

private:
    std::optional<std::pair<bool, bool>> _value;
    bool _corrupt = false;
};

class SafetyLatch {
public:
    explicit SafetyLatch(SafetyStore& store) : _store(store) {}

    void setMuteIntent(bool enabled) { _mute = enabled; }
    void setPrivacyIntent(bool enabled) { _privacy = enabled; }
    [[nodiscard]] bool muteIntent() const { return _mute; }
    [[nodiscard]] bool privacyIntent() const { return _privacy; }

    bool persist() { return _store.write(_mute, _privacy); }

    bool restoreFailClosed() {
        bool mute = true;
        bool privacy = true;
        if (!_store.read(mute, privacy)) {
            _mute = true;
            _privacy = true;
            return false;
        }
        _mute = mute;
        _privacy = privacy;
        return true;
    }

private:
    SafetyStore& _store;
    bool _mute = true;
    bool _privacy = true;
};

enum class SourceKind { kAudio, kVideo };

enum class SourceFault { kNone, kPermissionDenied, kPermissionRevoked, kBusy, kAmbiguous };

enum class DemandResult { kAcquired, kUnavailable, kContended };

class SourceBinding {
public:
    explicit SourceBinding(SourceKind kind) : _kind(kind) {}

    void bind(std::string id) {
        _id = std::move(id);
        _available = true;
        _ambiguous = false;
        _fault = SourceFault::kNone;
    }
    void lose() { _available = false; }
    void denyPermission() {
        _available = false;
        _ambiguous = false;
        _fault = SourceFault::kPermissionDenied;
    }
    void revokePermission() {
        _available = false;
        _ambiguous = false;
        _fault = SourceFault::kPermissionRevoked;
    }
    void markBusy() {
        _available = false;
        _ambiguous = false;
        _fault = SourceFault::kBusy;
    }
    void reconnectAmbiguous() {
        _available = false;
        _ambiguous = true;
        _fault = SourceFault::kAmbiguous;
    }

    [[nodiscard]] bool bound() const { return _id.has_value() && !_ambiguous; }
    [[nodiscard]] bool available() const { return bound() && _available; }
    [[nodiscard]] bool fallbackSelected() const { return false; }
    [[nodiscard]] SourceKind kind() const { return _kind; }
    [[nodiscard]] SourceFault fault() const { return _fault; }

private:
    SourceKind _kind;
    std::optional<std::string> _id;
    bool _available = false;
    bool _ambiguous = false;
    SourceFault _fault = SourceFault::kNone;
};

class DemandCoordinator {
public:
    DemandResult acquireResult(const SafetyLatch& safety, const SourceBinding& source) {
        if (source.fault() == SourceFault::kBusy) {
            _lastResult = DemandResult::kContended;
            return _lastResult;
        }
        if (!safety.muteIntent() || !safety.privacyIntent() || !source.available()) {
            _lastResult = DemandResult::kUnavailable;
            return _lastResult;
        }
        if (_owner) {
            _lastResult = DemandResult::kContended;
            return _lastResult;
        }
        _demandCount = 1;
        _owner = true;
        _lastResult = DemandResult::kAcquired;
        return _lastResult;
    }

    bool acquire(const SafetyLatch& safety, const SourceBinding& source) {
        return acquireResult(safety, source) == DemandResult::kAcquired;
    }

    void release() {
        if (_demandCount > 0) {
            --_demandCount;
        }
        if (_demandCount == 0) {
            _owner = false;
        }
    }

    [[nodiscard]] std::size_t demandCount() const { return _demandCount; }
    [[nodiscard]] bool ownsCapture() const { return _owner; }
    [[nodiscard]] DemandResult lastResult() const { return _lastResult; }

    bool acquireForVideo(const SafetyLatch& safety, const SourceBinding& source) {
        return acquire(safety, source);
    }

private:
    std::size_t _demandCount = 0;
    bool _owner = false;
    DemandResult _lastResult = DemandResult::kUnavailable;
};

template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : _capacity(capacity) {}

    bool push(T value) {
        if (_queue.size() >= _capacity) {
            return false;
        }
        _queue.push_back(std::move(value));
        return true;
    }

    std::optional<T> pop() {
        if (_queue.empty()) {
            return std::nullopt;
        }
        T value = std::move(_queue.front());
        _queue.pop_front();
        return value;
    }

    [[nodiscard]] std::size_t size() const { return _queue.size(); }
    [[nodiscard]] std::size_t capacity() const { return _capacity; }

private:
    std::size_t _capacity;
    std::deque<T> _queue;
};

template <typename T>
class LatestFrame {
public:
    void push(T value) { _value = std::move(value); }
    [[nodiscard]] std::size_t size() const { return _value.has_value() ? 1U : 0U; }
    [[nodiscard]] const std::optional<T>& value() const { return _value; }

private:
    std::optional<T> _value;
};

class RetryController {
public:
    explicit RetryController(std::size_t maxAttempts) : _maxAttempts(maxAttempts) {}
    bool tryAgain() {
        if (_cancelled || _attempts >= _maxAttempts) {
            return false;
        }
        ++_attempts;
        return true;
    }
    void cancel() { _cancelled = true; }
    [[nodiscard]] std::size_t attempts() const { return _attempts; }

private:
    std::size_t _maxAttempts;
    std::size_t _attempts = 0;
    bool _cancelled = false;
};

class ProcessorGuard {
public:
    explicit ProcessorGuard(bool downstreamSafe) : _downstreamSafe(downstreamSafe) {}
    [[nodiscard]] bool outputEnabled() const { return _downstreamSafe && !_faulted; }
    void fault() { _faulted = true; }

private:
    bool _downstreamSafe;
    bool _faulted = false;
};

}  // namespace axtual
