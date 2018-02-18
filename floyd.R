ed <- data.table::fread("r_connections_travel.csv")
st_master <- readRDS("master_r.rds")

## for R index have to be shifted by 1
ed <- ed %>% mutate(st_from = st_from+1, st_to = st_to+1)

## map to the master stations
ed_master <- ed %>% mutate(st_from = st_master[st_from], st_to = st_master[st_to])

ed_master_min <- ed_master %>% group_by(st_from,st_to) %>% summarise(min(weight)) %>% as.matrix()

mat_master <- matrix(data = NA, nrow = 810, ncol = 810)
mat_master[ed_master_min[,1:2]] <- ed_master_min[,3]

shortest_paths_master <- Rfast::floyd(mat_master)

saveRDS(object = shortest_paths_master, file = "adv_heuristic_r.rds")

writeLines(text = paste(nrow(shortest_paths_master),ncol(shortest_paths_master),sep=" "), con="adv_heuristic_cpp.csv")
write.table(x = shortest_paths_master, file = "adv_heuristic_cpp.csv", row.names = FALSE, col.names = FALSE, append = TRUE)
