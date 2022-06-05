#lang racket

(let ([lines (file->lines "data.txt")])
  (display "Header: ") (displayln (car lines))
  )
