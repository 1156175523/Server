CREATE TABLE
chatUser
(
    IP              VARCHAR(20)         NOT NULL,
    name            VARCHAR(32)         NOT NULL,
    fd              VARCHAR(8)          NOT NULL,
    status          VARCHAR(1)                  ,
    PRIMARY KEY (IP,fd)
);
