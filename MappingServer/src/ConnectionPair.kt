import Utility.Companion.SPLITER

data class ConnectionPair(
    val ipAddress: String,
    val port: Int
) {
    // Secondary constructor
    constructor(propertiesAsString: String) : this(
        propertiesAsString.split(SPLITER)[0],
        propertiesAsString.split(SPLITER)[1].toInt()
    )

    // toString
    override fun toString(): String {
        return "$ipAddress$SPLITER$port"
    }
}