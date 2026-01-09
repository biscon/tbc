import "game" for Game
import "random" for Random

class Level {
    // define a static vars on the metaclass
    static rng { __rng }      // getter
    static rng=(value) { __rng = value } // setter

    static init() {
        Level.rng = Random.new()
    }

    static onFirstTimeEntered() {
        //Game.setWeather("thunder")
        Game.setWeather("sunny")
        Game.setHourOfDay(12)
    }

    static onEnter() {
        System.print("Entering map!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        if(!Game.flag("fort_arlen_first_enter")) {
            onFirstTimeEntered()
            Game.setFlag("fort_arlen_first_enter", true)
        } else {
            System.print("not the first time entering")
            Game.setWeather("sunny")
            Game.setHourOfDay(7)
        }
        //System.print("flag = %(Game.flag("flag"))")
    }

    static onEnter_mallExit() {
        var answers = ["We're closed, GO AWAY!",
            "What part of we're closed, didn't you understand?.",
            "...",
            "Dumbass...",
            "Fuck off."]

        var hod = Game.getHourOfDay()
        if(hod >= 22 || hod <= 7) {
            if(!Game.flag("fort_arlen_mall_been_refused")) {
                Game.speechBubble("Come back tomorrow. We're like closed or something...", 520, 115, 5)
                Game.setFlag("fort_arlen_mall_been_refused", true)
                Game.setInt("for_arlen_exitAttempts", 0)
            } else {
                var idx = Game.getInt("for_arlen_exitAttempts")
                // Cap index at last element
                if (idx >= answers.count) idx = answers.count - 1
                Game.speechBubble(answers[idx], 520, 115, 5)
                idx = idx + 1
                Game.setInt("for_arlen_exitAttempts", idx)
            }
            return false
        } else {
            return true
        }
    }
}

Level.init()