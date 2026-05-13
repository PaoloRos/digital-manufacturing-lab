library(dplyr)
library(ggplot2)
library(readr)

df <-read_csv("output.csv") |> as_tibble() |> select(t_tot, lambda)

ggplot(df, aes(x=t_tot, y=lambda)) + 
  geom_line() +
  labs(x="Total Time (s)", y="Lambda")
