import "game" for Game

class Level {
    static onFirstTimeEntered() {
        Game.setWeather("thunder")
        Game.setHourOfDay(1)
    }

    static onEnter() {
        System.print("Entering map!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        if(!Game.flag("fort_arlen_first_enter")) {
            onFirstTimeEntered()
            Game.setFlag("fort_arlen_first_enter", true)
        }
        /*
        else {
            System.print("not the first time entering")
            Game.setWeather("sunny")
            Game.setHourOfDay(12)
        }
        */
        //System.print("flag = %(Game.flag("flag"))")
    }
}
