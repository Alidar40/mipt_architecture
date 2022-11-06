DROP TABLE IF EXISTS `ShoppingItem_ShoppingCart`;
DROP TABLE IF EXISTS `ShoppingCart`;
DROP TABLE IF EXISTS `ShoppingItem`;
DROP TABLE IF EXISTS `User`;

CREATE TABLE IF NOT EXISTS `User` (
`id` INT NOT NULL AUTO_INCREMENT, 
`first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`last_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`email` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
`password` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
`type` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
PRIMARY KEY (`id`), KEY `fn` (`first_name`), KEY `ln` (`last_name`)
);

CREATE TABLE IF NOT EXISTS `ShoppingItem` (
`id` INT NOT NULL AUTO_INCREMENT,
`sku` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`price` DECIMAL(10, 2),
PRIMARY KEY (`id`), UNIQUE KEY `s` (`sku`), KEY `n` (`name`)
);

CREATE TABLE IF NOT EXISTS `ShoppingCart` (
`id` INT NOT NULL AUTO_INCREMENT,
`user_id` INT NOT NULL,
`created` DATETIME DEFAULT CURRENT_TIMESTAMP NOT NULL,
PRIMARY KEY (`id`),
FOREIGN KEY (`user_id`) REFERENCES `User`(`id`) ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS `ShoppingItem_ShoppingCart` (
`id` INT NOT NULL AUTO_INCREMENT,
`shopping_item_id` INT NOT NULL,
`shopping_cart_id` INT NOT NULL,
PRIMARY KEY (`id`),
FOREIGN KEY (`shopping_item_id`) REFERENCES `ShoppingItem`(`id`) ON UPDATE CASCADE ON DELETE CASCADE,
FOREIGN KEY (`shopping_cart_id`) REFERENCES `ShoppingCart`(`id`) ON UPDATE CASCADE ON DELETE CASCADE
);

-- https://generatedata.com/generator
INSERT INTO `User`(`first_name`, `last_name`, `email`, `password`, `type`) VALUES
("Tyler","Black","in.mi.pede@aol.net","TOT40XUH1HT","aliquam"),
("August","Singleton","tincidunt.congue@icloud.edu","DHF83GLI6UV","fames"),
("Davis","Flores","in@protonmail.org","IPF32UED0GJ","libero."),
("Berk","Dotson","leo.in@outlook.couk","JLD67NYW1YL","sem."),
("Russell","Blackwell","luctus.felis.purus@google.edu","PNK75JLG2XB","sem");

INSERT INTO `ShoppingItem`(`sku`, `name`, `price`) VALUES
("8817","Mauris magna.",57262),
("5746","feugiat. Lorem",68923),
("1081","fringilla",83557),
("6037","fringilla mi",2975),
("6820","gravida",56687);

-- INSERT INTO `ShoppingCart`(`user_id`) VALUES
-- (1),
-- (2),
-- (3);
INSERT INTO `ShoppingCart`(`user_id`, `created`) VALUES
(1, '2022-11-01 14:00:00'),
(2, '2022-11-02 09:00:00'),
(3, '2022-11-02 19:00:00');

INSERT INTO `ShoppingItem_ShoppingCart`(`shopping_item_id`, `shopping_cart_id`) VALUES
(1, 1),
(2, 1),
(3, 1),
(4, 2),
(2, 2),
(1, 3);