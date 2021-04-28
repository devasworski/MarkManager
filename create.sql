-- -----------------------------------------------------
-- Schema MarkManager
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `MarkManager` DEFAULT CHARACTER SET utf8 ;
USE `MarkManager` ;

-- -----------------------------------------------------
-- Table `MarkManager`.`modules`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `MarkManager`.`modules` (
  `idmodules` INT NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(45) NOT NULL,
  `active` TINYINT NOT NULL,
  PRIMARY KEY (`idmodules`),
  UNIQUE INDEX `idmodules_UNIQUE` (`idmodules` ASC));

-- -----------------------------------------------------
-- Table `MarkManager`.`students`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `MarkManager`.`students` (
  `idstudents` INT NOT NULL AUTO_INCREMENT,
  `Name` VARCHAR(80) NOT NULL,
  `active` TINYINT NULL,
  PRIMARY KEY (`idstudents`));

-- -----------------------------------------------------
-- Table `MarkManager`.`login`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `MarkManager`.`login` (
  `name` VARCHAR(45) NOT NULL,
  `email` VARCHAR(45) NOT NULL,
  `password` VARCHAR(65) NOT NULL,
  `fullname` VARCHAR(50) NOT NULL,
  `active` TINYINT NOT NULL,
  PRIMARY KEY (`name`),
  UNIQUE INDEX `email_UNIQUE` (`email` ASC),
  UNIQUE INDEX `name_UNIQUE` (`name` ASC));

-- -----------------------------------------------------
-- Table `MarkManager`.`students_has_modules`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `MarkManager`.`students_has_modules` (
  `students_idstudents` INT NOT NULL,
  `modules_idmodules` INT NOT NULL,
  `grade` INT NULL,
  PRIMARY KEY (`students_idstudents`, `modules_idmodules`),
  INDEX `fk_students_has_modules_modules1_idx` (`modules_idmodules` ASC),
  INDEX `fk_students_has_modules_students1_idx` (`students_idstudents` ASC),
  CONSTRAINT `fk_students_has_modules_students1`
    FOREIGN KEY (`students_idstudents`)
    REFERENCES `MarkManager`.`students` (`idstudents`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_students_has_modules_modules1`
    FOREIGN KEY (`modules_idmodules`)
    REFERENCES `MarkManager`.`modules` (`idmodules`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);

-- -----------------------------------------------------
-- Table `MarkManager`.`modules_has_lectures`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `MarkManager`.`modules_has_lectures` (
  `modules_idmodules` INT NOT NULL,
  `login_name` VARCHAR(45) NOT NULL,
  PRIMARY KEY (`modules_idmodules`, `login_name`),
  INDEX `fk_modules_has_lectures_modules1_idx` (`modules_idmodules` ASC),
  INDEX `fk_modules_has_lectures_login1_idx` (`login_name` ASC),
  CONSTRAINT `fk_modules_has_lectures_modules1`
    FOREIGN KEY (`modules_idmodules`)
    REFERENCES `MarkManager`.`modules` (`idmodules`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_modules_has_lectures_login1`
    FOREIGN KEY (`login_name`)
    REFERENCES `MarkManager`.`login` (`name`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);

-- -----------------------------------------------------
-- Table `MarkManager`.`Sessions`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `MarkManager`.`Sessions` (
  `SessionID` VARCHAR(65) NOT NULL,
  `fk_user` VARCHAR(45) NOT NULL,
  `expires` DATETIME NOT NULL,
  `IP` VARCHAR(15) NOT NULL,
  `active` TINYINT NOT NULL,
  PRIMARY KEY (`SessionID`),
  UNIQUE INDEX `SessionID_UNIQUE` (`SessionID` ASC),
  INDEX `name_idx` (`fk_user` ASC),
  CONSTRAINT `name`
    FOREIGN KEY (`fk_user`)
    REFERENCES `MarkManager`.`login` (`name`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);

-- -----------------------------------------------------
-- Table `MarkManager`.`TOTP`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `MarkManager`.`TOTP` (
  `seed` VARCHAR(56) NOT NULL,
  `fk_user` VARCHAR(45) NOT NULL,
  `active` TINYINT NOT NULL,
  PRIMARY KEY (`fk_user`),
  UNIQUE INDEX `seed_UNIQUE` (`seed` ASC),
  CONSTRAINT `fk_user`
    FOREIGN KEY (`fk_user`)
    REFERENCES `MarkManager`.`login` (`name`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);

-- -----------------------------------------------------
-- Table `MarkManager`.`2FA`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `MarkManager`.`2FA` (
  `code` INT NOT NULL,
  `fk_user` VARCHAR(45) NOT NULL,
  `id` INT NOT NULL AUTO_INCREMENT,
  `expires` DATETIME NULL,
  INDEX `fk_user_idx` (`fk_user` ASC),
  PRIMARY KEY (`id`),
  CONSTRAINT `fk_user_2fa`
    FOREIGN KEY (`fk_user`)
    REFERENCES `MarkManager`.`login` (`name`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);

INSERT INTO login(name, password, active) VALUES("admin","root","0");

CREATE USER 'user'@'localhost' IDENTIFIED BY 'd9pifetoyesad2cekipoyolis';
GRANT SELECT ON MarkManager.modules_has_lectures TO 'user'@'localhost';
GRANT SELECT ON MarkManager.students_has_modules TO 'user'@'localhost';
GRANT UPDATE(grade) ON MarkManager.students_has_modules TO 'user'@'localhost';
GRANT SELECT ON MarkManager.students TO 'user'@'localhost';
GRANT SELECT ON MarkManager.modules TO 'user'@'localhost';

CREATE USER 'admin'@'localhost' IDENTIFIED BY 'nocikocofot9com4cif1boq6t';
GRANT SELECT ON MarkManager.modules_has_lectures TO 'admin'@'localhost';
GRANT SELECT ON MarkManager.students_has_modules TO 'admin'@'localhost';
GRANT INSERT ON MarkManager.modules_has_lectures TO 'admin'@'localhost';
GRANT INSERT ON MarkManager.students_has_modules TO 'admin'@'localhost';
GRANT DELETE ON MarkManager.modules_has_lectures TO 'admin'@'localhost';
GRANT DELETE ON MarkManager.students_has_modules TO 'admin'@'localhost';
GRANT SELECT ON MarkManager.students TO 'admin'@'localhost';
GRANT SELECT ON MarkManager.modules TO 'admin'@'localhost';
GRANT SELECT(name,fullname,active) ON MarkManager.login TO 'admin'@'localhost';
GRANT INSERT ON MarkManager.students TO 'admin'@'localhost';
GRANT UPDATE(active) ON MarkManager.students TO 'admin'@'localhost';
GRANT INSERT ON MarkManager.modules TO 'admin'@'localhost';
GRANT UPDATE(active) ON MarkManager.modules TO 'admin'@'localhost';

CREATE USER 'login'@'localhost' IDENTIFIED BY 'n9ras1yay4aey8yoterec2vex';
GRANT SELECT ON MarkManager.TOTP TO 'login'@'localhost';
GRANT SELECT ON MarkManager.2FA TO 'login'@'localhost';
GRANT INSERT ON MarkManager.2FA TO 'login'@'localhost';
GRANT SELECT ON MarkManager.login TO 'login'@'localhost';
GRANT SELECT ON MarkManager.Sessions TO 'login'@'localhost';
GRANT INSERT ON MarkManager.Sessions TO 'login'@'localhost';
GRANT UPDATE(active) ON MarkManager.Sessions TO 'login'@'localhost';

CREATE USER 'stateful'@'localhost' IDENTIFIED BY 'dajuyihobumasopin7qefiroa';
GRANT SELECT ON MarkManager.Sessions TO 'stateful'@'localhost';

CREATE USER 'register'@'localhost' IDENTIFIED BY 'qaciy0t2kif5cul5d5quwanus';
GRANT SELECT ON MarkManager.login TO 'register'@'localhost';
GRANT INSERT ON MarkManager.login TO 'register'@'localhost';
GRANT UPDATE ON MarkManager.login TO 'register'@'localhost';
GRANT UPDATE(active) ON MarkManager.login TO 'register'@'localhost';
GRANT SELECT ON MarkManager.TOTP TO 'register'@'localhost';
GRANT INSERT ON MarkManager.TOTP TO 'register'@'localhost';
GRANT UPDATE(active,seed) ON MarkManager.TOTP TO 'register'@'localhost';
GRANT SELECT ON MarkManager.2FA TO 'register'@'localhost';
GRANT INSERT ON MarkManager.2FA TO 'register'@'localhost';