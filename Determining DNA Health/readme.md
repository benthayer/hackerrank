These are my solution attempts for the Determining DNA Health problem
https://www.hackerrank.com/challenges/determining-dna-health/problem

I was the 357th person to solve this problem.

In v0, I used a map to lookup the extra things I needed. This solved a few test cases, but timed out on many.

In v1, I switched from using a map to using a 26-ary tree for more efficiant lookups. This solved all test cases except 13 and 14, which timed out.

v2 is the same as v1, but I add a binary tree to calculate the health of a given gene. This is particularly useful on problems 13 and 14 because of the small character set, which causes a lot of overlap between genes. The genes that were 2 long had around 750 instances, 3 long had around 200 each. Since these are also the most common, the reduction of 750 to log_2(750)=9.5 and 200 to log_2(200)=7.6 resulted in a speedup of 6x from 25s to 4.1s for the test suite on my machine.