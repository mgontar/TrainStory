con_st <- data.table::fread("r_connections_travel.csv")
con_wait <- data.table::fread("r_connections_wait.csv")

con_wait <- con_wait %>% mutate(weight = weight/60)

writeLines(text = as.character(nrow(con_st)), con = "connections_train_cpp.txt")
write.table(x = con_st, file = "connections_train_cpp.txt", row.names = FALSE, col.names = FALSE, append = TRUE)

writeLines(text = as.character(nrow(con_wait)), con = "connections_wait_cpp.txt")
write.table(x = con_wait, file = "connections_wait_cpp.txt", row.names = FALSE, col.names = FALSE, append = TRUE)

