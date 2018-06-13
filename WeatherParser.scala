import xml.XML

object WeatherParser extends App {
  val xml = XML.loadFile("ndfd.xml")
  val temperatures = xml \\ "data" \\ "parameters" \ "temperature"
  val maxTemps = temperatures.find(t => (t \ "@type").text == "maximum").get.flatMap(t => (t \ "value").map(_.text.toInt))
  val minTemps = temperatures.find(t => (t \ "@type").text == "minimum").get.flatMap(t => (t \ "value").map(_.text.toInt))
  println("max temps for the next 4 days")
  maxTemps.foreach(println)
  println("min temps for the next 4 days")
  minTemps.foreach(println)
}
