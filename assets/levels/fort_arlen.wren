import "game" for Game

class Level {
    static onEnter() {
        System.print("Entering map!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        //Game.setFlag("flag", true)
        System.print("flag = %(Game.flag("flag"))")
    }
}
