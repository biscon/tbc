import "game" for Game
import "random" for Random

class Level {
    // define a static vars on the metaclass
    static rng { __rng }      // getter
    static rng=(value) { __rng = value } // setter

    static init() {
        Level.rng = Random.new()
    }

    static onEnter() {
        System.print("Entering tent!!!!!!")

    }

    static onPimpTentDialogueTrigger() {
        Game.startDialogue(1)
    }
}

Level.init()