# Assignment 4

> Why there are two question A?

## Answer

### A

![Process Tree](./lab4_a_process_tree.png)

\newpage

### B

![Question B result](./lab4_b.png)

### Second A

![Question Second A result](./lab4_a.png)

\newpage

## Attachments

### Question A Procedure Mermaid

![Procedure Flow Chart](./lab4_a_flow.png)

``` mermaid
flowchart TD
    P0(["x = -11, y = -22, z = -33"])
    P1(["x > 0, y = -22, z = -33"])
    P2(["x = 0, y = -22, z = -33"])
    P3(["x > 0, y > 0, z = -33"])
    P4(["x > 0, y = 0, z = -33"])
    P5(["x = 0, y > 0, z = -33"])
    P6(["x = 0, y = 0, z = -33"])
    P7(["x > 0, y = 0, z > 0"])
    P8(["x = 0, y = 0, z = 0"])

    P0 -->|"x = fork()"| P1
    P0 -->|"x = fork()"| P2
    P1 -->|"y = fork()"| P3
    P1 -->|"y = fork()"| P4
    P2 -->|"y = fork()"| P5
    P2 -->|"y = fork()"| P6
    P4 -->|"if(!y) z = fork()"| P7
    P6 -->|"if(!y) z = fork()"| P8


    classDef process fill:#d3edf3,stroke:#222,color:#111;
    class P0,P1,P2,P3,P4,P5,P6,P7,P8 process;
```

### Question A Process Tree Mermaid

``` mermaid
flowchart TD
    P0(["P0<br/>x > 0, y > 0, z = -33"])
    P1(["P1<br/>x = 0, y > 0, z = -33"])
    P2(["P2<br/>x > 0, y = 0, z > 0"])
    P3(["P3<br/>x = 0, y = 0, z > 0"])
    P4(["P4<br/>x > 0, y = 0, z = 0"])
    P5(["P5<br/>x = 0, y = 0, z = 0"])

    P0 -->|"x = fork()"| P1
    P0 -->|"y = fork()"| P2
    P1 -->|"y = fork()"| P3
    P2 -->|"z = fork()"| P4
    P3 -->|"z = fork()"| P5

    classDef process fill:#d3edf3,stroke:#222,color:#111;
    class P0,P1,P2,P3,P4,P5 process;
```
