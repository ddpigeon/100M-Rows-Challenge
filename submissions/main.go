package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

type Stats struct {
	Min   float64
	Max   float64
	Sum   float64
	Count int
}

func main() {
	file, err := os.Open("measurements.txt")
	if err != nil {
		fmt.Println("Error opening file:", err)
		return
	}
	defer file.Close()

	data := make(map[string]*Stats)
	scanner := bufio.NewScanner(file)

	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ";")
		if len(parts) != 2 {
			fmt.Println("Skipping invalid line:", line)
			continue
		}

		place := strings.TrimSpace(parts[0])
		measurement, err := strconv.ParseFloat(strings.TrimSpace(parts[1]), 64)
		if err != nil {
			fmt.Println("Skipping invalid measurement:", parts[1])
			continue
		}

		if _, exists := data[place]; !exists {
			data[place] = &Stats{Min: measurement, Max: measurement}
		}

		stats := data[place]
		if measurement < stats.Min {
			stats.Min = measurement
		}
		if measurement > stats.Max {
			stats.Max = measurement
		}
		stats.Sum += measurement
		stats.Count++
	}

	if err := scanner.Err(); err != nil {
		fmt.Println("Error reading file:", err)
		return
	}

	for place, stats := range data {
		average := stats.Sum / float64(stats.Count)
		fmt.Printf("Place: %s\nMin: %.2f\nMax: %.2f\nAverage: %.10f\n\n", place, stats.Min, stats.Max, average)
	}
}
