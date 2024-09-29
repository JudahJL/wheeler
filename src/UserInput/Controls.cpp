#include "UserInput/Controls.h"
#include "Wheeler/Wheeler.h"
#include "Config.h"

void Controls::Init()
{
	BindAllInputsFromConfig();
}

void Controls::BindAllInputsFromConfig()
{
	std::lock_guard lock(_lock);
	_keyFunctionMapDown.clear();
	_keyFunctionMapDownGamepad.clear();
	_keyFunctionMapUp.clear();
	_keyFunctionMapUpGamepad.clear();
	using namespace Config::InputBindings;
	{
		using namespace MKB;
		for (const auto& [fst, snd] : std::array{
				 std::make_pair( activatePrimary, &Wheeler::ActivateHoveredEntryPrimary ),
				 std::make_pair( activateSecondary, &Wheeler::ActivateHoveredEntrySecondary ),
				 std::make_pair( addWheel, &Wheeler::AddWheel),
				 std::make_pair( addEmptyEntry, &Wheeler::AddEmptyEntryToCurrentWheel ),
				 std::make_pair( moveEntryForward, &Wheeler::MoveEntryForwardInCurrentWheel ),
				 std::make_pair( moveEntryBack, &Wheeler::MoveEntryBackInCurrentWheel ),
				 std::make_pair( moveWheelForward, &Wheeler::MoveWheelForward ),
				 std::make_pair( moveWheelBack, &Wheeler::MoveWheelBack ),
				 std::make_pair( nextWheel, &Wheeler::NextWheel ),
				 std::make_pair( prevWheel, &Wheeler::PrevWheel ),
				 std::make_pair( toggleWheel, &Wheeler::ToggleWheeler ),
				 std::make_pair( prevItem, &Wheeler::PrevItemInEntry ),
				 std::make_pair( nextItem, &Wheeler::NextItemInEntry )
			}) {
			bindInput(fst, snd, true, false);
		}
		bindInput(toggleWheel, &Wheeler::CloseWheelerIfOpenedLongEnough, false, false);
	}
	{
		using namespace GamePad;
		for (const auto& [fst, snd] : std::array{
				 std::make_pair( activatePrimary, &Wheeler::ActivateHoveredEntryPrimary ),
				 std::make_pair( activateSecondary, &Wheeler::ActivateHoveredEntrySecondary ),
				 std::make_pair( addWheel, &Wheeler::AddWheel ),
				 std::make_pair( addEmptyEntry, &Wheeler::AddEmptyEntryToCurrentWheel ),
				 std::make_pair( moveEntryForward, &Wheeler::MoveEntryForwardInCurrentWheel ),
				 std::make_pair( moveEntryBack, &Wheeler::MoveEntryBackInCurrentWheel ),
				 std::make_pair( moveWheelForward, &Wheeler::MoveWheelForward ),
				 std::make_pair( moveWheelBack, &Wheeler::MoveWheelBack ),
				 std::make_pair( nextWheel, &Wheeler::NextWheel ),
				 std::make_pair( prevWheel, &Wheeler::PrevWheel ),
				 std::make_pair( toggleWheel, &Wheeler::ToggleWheeler ),
				 std::make_pair( toggleWheelIfNotInInventory, &Wheeler::ToggleWheelIfNotInInventory ),
				 std::make_pair( toggleWheelIfInInventory, &Wheeler::ToggleWheelIfInInventory ),
				 std::make_pair( prevItem, &Wheeler::PrevItemInEntry ),
				 std::make_pair( nextItem, &Wheeler::NextItemInEntry ) }) {
			bindInput(fst, snd, true, true);
		}
		bindInput(toggleWheel, &Wheeler::CloseWheelerIfOpenedLongEnough, false, true);
		bindInput(toggleWheelIfInInventory, &Wheeler::CloseWheelerIfOpenedLongEnoughIfInInventory, false, true);
		bindInput(toggleWheelIfNotInInventory, &Wheeler::CloseWheelerIfOpenedLongEnoughIfNotInInventory, false, true);
	}

}
void Controls::bindInput(const KeyId key, const FunctionPtr func, const bool isDown, const bool isGamePad) {
    (isDown ? isGamePad ? _keyFunctionMapDownGamepad : _keyFunctionMapDown : isGamePad ? _keyFunctionMapUpGamepad :
                                                                                         _keyFunctionMapUp)[key] = func;
}

bool Controls::IsKeyBound(const KeyId key)
{
	const std::lock_guard lock(_lock);
	return _keyFunctionMapDown.contains(key) || _keyFunctionMapUp.contains(key) || _keyFunctionMapDownGamepad.contains(key) || _keyFunctionMapUpGamepad.contains(key);
}


bool Controls::Dispatch(const KeyId key, const bool isDown, const bool isGamePad)
{
	const std::lock_guard lock(_lock);
	if (isDown) {
		if (isGamePad) {
			if (_keyFunctionMapDownGamepad.contains(key)) {
                _keyFunctionMapDownGamepad[key]();
				return true;
			}
		} else {
			if (_keyFunctionMapDown.contains(key)) {
				_keyFunctionMapDown[key]();
				return true;
			}
		}

	} else {
		if (isGamePad) {
			if (_keyFunctionMapUpGamepad.contains(key)) {
				_keyFunctionMapUpGamepad[key]();
				return true;
			}
		} else {
			if (_keyFunctionMapUp.contains(key)) {
				_keyFunctionMapUp[key]();
				return true;
			}
		}
	}
	return false;
}
