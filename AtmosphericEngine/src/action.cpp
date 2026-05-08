#include "action.hpp"
#include "animator_2d.hpp"// For AnimationClip definition
#include "sprite_component.hpp"
#include <algorithm>

// --- ActionInterval ---

ActionInterval::ActionInterval(float duration) : FiniteTimeAction(duration), _elapsed(0), _firstTick(true) {
    if (_duration == 0) _duration = FLT_EPSILON;
}

bool ActionInterval::IsDone() const {
    return _elapsed >= _duration;
}

void ActionInterval::Step(float dt) {
    if (_firstTick) {
        _firstTick = false;
        _elapsed = 0;
    } else {
        _elapsed += dt;
    }

    float t = std::min(1.0f, _elapsed / _duration);
    Update(t);
}

void ActionInterval::StartWithTarget(GameObject* target) {
    Action::StartWithTarget(target);
    _elapsed = 0;
    _firstTick = true;
}

// --- MoveTo ---

MoveTo::MoveTo(float duration, const glm::vec3& position) : ActionInterval(duration), _endPosition(position) {
}

void MoveTo::StartWithTarget(GameObject* target) {
    ActionInterval::StartWithTarget(target);
    _startPosition = target->GetPosition();
    _delta = _endPosition - _startPosition;
}

void MoveTo::Update(float t) {
    if (_target) {
        _target->SetPosition(_startPosition + _delta * t);
    }
}

// --- MoveBy ---

MoveBy::MoveBy(float duration, const glm::vec3& deltaPosition) : ActionInterval(duration), _delta(deltaPosition) {
}

void MoveBy::StartWithTarget(GameObject* target) {
    ActionInterval::StartWithTarget(target);
    _startPosition = target->GetPosition();
    _previousPosition = _startPosition;
}

void MoveBy::Update(float t) {
    if (_target) {
        glm::vec3 currentPos = _target->GetPosition();
        glm::vec3 diff = currentPos - _previousPosition;
        _startPosition = _startPosition + diff;

        glm::vec3 newPos = _startPosition + _delta * t;
        _target->SetPosition(newPos);
        _previousPosition = newPos;
    }
}

// --- RotateTo ---

RotateTo::RotateTo(float duration, const glm::vec3& rotation) : ActionInterval(duration), _dstRotation(rotation) {
}

void RotateTo::StartWithTarget(GameObject* target) {
    ActionInterval::StartWithTarget(target);
    _startRotation = target->GetRotation();
    _diffRotation = _dstRotation - _startRotation;
}

void RotateTo::Update(float t) {
    if (_target) {
        _target->SetRotation(_startRotation + _diffRotation * t);
    }
}

// --- RotateBy ---

RotateBy::RotateBy(float duration, const glm::vec3& deltaRotation) : ActionInterval(duration), _delta(deltaRotation) {
}

void RotateBy::StartWithTarget(GameObject* target) {
    ActionInterval::StartWithTarget(target);
    _startRotation = target->GetRotation();
}

void RotateBy::Update(float t) {
    // Simple implementation, might drift if modified externally
    if (_target) {
        _target->SetRotation(_startRotation + _delta * t);
    }
}

// --- ScaleTo ---

ScaleTo::ScaleTo(float duration, const glm::vec3& scale) : ActionInterval(duration), _endScale(scale) {
}

void ScaleTo::StartWithTarget(GameObject* target) {
    ActionInterval::StartWithTarget(target);
    _startScale = target->GetScale();
    _delta = _endScale - _startScale;
}

void ScaleTo::Update(float t) {
    if (_target) {
        _target->SetScale(_startScale + _delta * t);
    }
}

// --- Sequence ---

Sequence::Sequence(const std::vector<FiniteTimeAction*>& actions) : ActionInterval(0), _actions(actions) {
    float totalDuration = 0;
    for (auto action : _actions) {
        totalDuration += action->GetDuration();
    }
    _duration = totalDuration;
}

Sequence::~Sequence() {
    for (auto action : _actions) {
        delete action;
    }
}

void Sequence::StartWithTarget(GameObject* target) {
    ActionInterval::StartWithTarget(target);
    _currentActionIndex = 0;
    if (!_actions.empty()) {
        _currentAction = _actions[0];
        _currentAction->StartWithTarget(target);
    } else {
        _currentAction = nullptr;
    }
}

void Sequence::Step(float dt) {
    if (_currentAction) {
        _currentAction->Step(dt);
        if (_currentAction->IsDone()) {
            _currentActionIndex++;
            if (_currentActionIndex < _actions.size()) {
                _currentAction = _actions[_currentActionIndex];
                _currentAction->StartWithTarget(_target);
            } else {
                _currentAction = nullptr;
            }
        }
    }
}

void Sequence::Update(float t) {
    // Not used for Sequence
}

// --- CallFunc ---

CallFunc::CallFunc(std::function<void()> func) : FiniteTimeAction(0), _function(func) {
}

void CallFunc::StartWithTarget(GameObject* target) {
    Action::StartWithTarget(target);
    if (_function) _function();
    _done = true;
}

void CallFunc::Step(float dt) {
    // Executed in StartWithTarget
}

// --- RepeatForever ---

RepeatForever::RepeatForever(ActionInterval* action) : _innerAction(action) {
}

RepeatForever::~RepeatForever() {
    delete _innerAction;
}

void RepeatForever::StartWithTarget(GameObject* target) {
    Action::StartWithTarget(target);
    _innerAction->StartWithTarget(target);
}

void RepeatForever::Step(float dt) {
    _innerAction->Step(dt);
    if (_innerAction->IsDone()) {
        _innerAction->StartWithTarget(_target);
    }
}

// --- Animate ---

Animate::Animate(const AnimationClip& clip) : ActionInterval(0), _clip(new AnimationClip(clip)) {
    float totalDuration = 0;
    for (const auto& frame : _clip->frames) {
        totalDuration += frame.duration;
        _splitTimes.push_back(totalDuration);
    }
    _duration = totalDuration;
    // Normalize split times
    if (_duration > 0) {
        for (float& t : _splitTimes) {
            t /= _duration;
        }
    }
}

void Animate::StartWithTarget(GameObject* target) {
    ActionInterval::StartWithTarget(target);
    _sprite = target->GetComponent<SpriteComponent>();
    _currentFrame = 0;
}

void Animate::Update(float t) {
    if (!_sprite || !_clip) return;

    // Find frame based on t
    int frameIndex = 0;
    for (size_t i = 0; i < _splitTimes.size(); ++i) {
        if (t <= _splitTimes[i]) {
            frameIndex = i;
            break;
        }
    }
    if (t >= 1.0f) frameIndex = (int)_splitTimes.size() - 1;

    if (frameIndex != _currentFrame) {
        _currentFrame = frameIndex;
        const auto& frame = _clip->frames[_currentFrame];
        _sprite->SetUVs(frame.uvMin, frame.uvMax);
    }
}
