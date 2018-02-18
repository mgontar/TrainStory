id_name <- data.table::fread("r_stations_events.csv",nrows = 810)
id_name <- id_name %>% select(name)

writeLines(text = as.character(nrow(id_name)), con = "station_names_c.txt")
write.table(x = id_name, file = "station_names_c.txt", row.names = FALSE, col.names = FALSE, append = TRUE, quote = FALSE)
