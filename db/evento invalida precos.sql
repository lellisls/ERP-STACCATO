DROP PROCEDURE IF EXISTS InvalidateExpired; 

DELIMITER //
CREATE PROCEDURE InvalidateExpired()
BEGIN 

	INSERT INTO Log (usuario, tipoOperacao, data, log) VALUES ('SISTEMA', 'Manutenção', NOW(), 'Descontinuando produtos vencidos.');

	UPDATE Produto_has_Preco 
SET 
    expirado = TRUE
WHERE
    validadeFim < CURDATE();
    
    -- se nao tiver preço valido, marca produto como descontinuado                
	UPDATE Produto 
SET 
    descontinuado = TRUE
WHERE
    idProduto NOT IN (SELECT 
            idProduto
        FROM
            Produto_has_Preco
        WHERE
            expirado = FALSE)
        AND descontinuado = FALSE;

-- copia o preço valido para produto
	UPDATE Produto AS p,
    produto_has_preco AS prc 
SET 
    p.precovenda = prc.preco
WHERE
    p.idProduto = prc.idProduto
        AND prc.expirado = FALSE
        AND p.descontinuado = FALSE;
        
END //
DELIMITER ; 

DROP EVENT IF EXISTS invalidate_all_expired;
CREATE EVENT invalidate_all_expired
    ON SCHEDULE
		EVERY 1 DAY
		STARTS (TIMESTAMP(CURRENT_DATE) + INTERVAL 1 DAY + INTERVAL 6 HOUR)
    DO
      CALL InvalidateExpired();