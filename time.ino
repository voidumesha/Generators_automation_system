
String DayOfTheWeek(uint8_t Day) {
  String DayText;
  if (Day == 1) DayText = "Mon";
  if (Day == 2) DayText = "Tue";
  if (Day == 3) DayText = "Wed";
  if (Day == 4) DayText = "Thu";
  if (Day == 5) DayText = "Fri";
  if (Day == 6) DayText = "Sat";
  if (Day == 7) DayText = "Sun";
  return DayText;
  
}



String DayMonthYear(uint8_t Day, uint8_t Month, uint16_t Year) {
  String DayMonthYearText;
  if (Month == 1) DayMonthYearText = "JAN ";
  if (Month == 2) DayMonthYearText = "FEB ";
  if (Month == 3) DayMonthYearText = "MAR ";
  if (Month == 4) DayMonthYearText = "APR ";
  if (Month == 5) DayMonthYearText = "MAY ";
  if (Month == 6) DayMonthYearText = "JUN ";
  if (Month == 7) DayMonthYearText = "JUL ";
  if (Month == 8) DayMonthYearText = "AUG ";
  if (Month == 9) DayMonthYearText = "SEP ";
  if (Month == 10) DayMonthYearText = "OCT ";
  if (Month == 11) DayMonthYearText = "NOV ";
  if (Month == 12) DayMonthYearText = "DEC ";

  DayMonthYearText = DayMonthYearText + Day;
  if (Day == 1) DayMonthYearText = DayMonthYearText + "st ";
  if (Day == 2) DayMonthYearText = DayMonthYearText + "nd ";
  if (Day > 2) DayMonthYearText = DayMonthYearText + "th ";

  DayMonthYearText = DayMonthYearText + Year;

  return DayMonthYearText;
}

String AddLeadingZero(uint8_t x) {
  String AddLeadingZeroText;
  if (x < 10) AddLeadingZeroText = "0";
  else AddLeadingZeroText = "";
  AddLeadingZeroText = AddLeadingZeroText + x;
  return AddLeadingZeroText;
}

String CurrentTime(uint8_t h, uint8_t i) {
  String CurrentTimeText = "";
  CurrentTimeText = CurrentTimeText + AddLeadingZero(h) + ":" + AddLeadingZero(i);
  return CurrentTimeText;
}
