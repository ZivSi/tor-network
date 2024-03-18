import java.math.BigInteger
import java.security.MessageDigest

class Utility {
    companion object {
        const val SPLITER = "::::"
        private val animals = arrayOf(
            "dog",
            "cat",
            "bird",
            "fish",
            "hamster",
            "rabbit",
            "turtle",
            "horse",
            "snake",
            "lizard",
            "frog",
            "chicken",
            "duck",
            "goat",
            "cow",
            "pig",
            "sheep",
            "elephant",
            "giraffe",
            "zebra",
            "gorilla",
            "monkey",
            "kangaroo",
            "koala",
            "panda",
            "lion",
            "tiger",
            "bear",
            "wolf",
            "fox",
            "deer",
            "moose",
            "elk",
            "buffalo",
            "camel",
            "llama",
            "alpaca",
            "turkey",
            "ostrich",
            "emu",
            "crocodile",
            "alligator",
            "octopus",
            "squid",
            "crab",
            "lobster",
            "shrimp",
            "snail",
            "fly",
            "ant",
        )
        private val colors = arrayOf(
            "red",
            "orange",
            "yellow",
            "green",
            "blue",
            "purple",
            "pink",
            "brown",
            "black",
            "white",
            "gray",
            "silver",
            "gold"
        )

        fun splitRsaKey(rsaKey: String): Pair<ULong, ULong> {
            val split = rsaKey.split(SPLITER)

            return Pair(split[0].toULong(), split[1].toULong())
        }

        fun generateUsername(): String {
            return "${colors.random()} ${animals.random()}"
        }

        fun hashStr(input: String): String {
            val digest = MessageDigest.getInstance("SHA-256")
            val hashBytes = digest.digest(input.toByteArray())
            val hashed = BigInteger(1, hashBytes).toString(16).padStart(64, '0')
            return hashed
        }
    }
}