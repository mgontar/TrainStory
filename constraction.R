st <- data.table::fread("r_stations_events.csv")
st <- st %>% mutate(master = sub(pattern="([0-9]+)\\..*",replacement="\\1",x=name))
st$master[1:810] <- as.character(0:809)
st <- st %>% mutate(master = as.integer(master))

master_cpp <- st %>% select(master)
write.table(x = rbind(nrow(master_cpp),master_cpp), file = "master_cpp.csv", row.names = FALSE, col.names = FALSE)

master_r <- master_cpp %>% mutate(master = master+1) %>% pull(master)
saveRDS(object = master_r, file = "master_r.rds")
