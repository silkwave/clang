/* shm.sql
 * 목적:
 * - main.c의 출력 예시(<<TaskHaltStat 전체값(0/1/X)>>)를
 *   오라클에서 재현하기 위한 DDL/DML/SELECT 예제
 *
 * 개요:
 * - APC_SHM_ALL 단일 테이블에 3가지 종류의 데이터를 저장한다.
 *   - ROW_KIND='M' : 매체 인덱스(0~49)와 한글명
 *   - ROW_KIND='J' : 업무 인덱스(0~49)와 한글명(본 샘플에서는 미사용)
 *   - ROW_KIND='T' : TaskHaltStat 셀(매체/업무 조합) 값
 */

/* =========================================================
 * DDL: 테이블 생성
 * ========================================================= */

-- Optional: drop if exists (ignore error if missing)
-- DROP TABLE APC_SHM_ALL PURGE;

CREATE TABLE APC_SHM_ALL (
  HOST_ID     NUMBER(2)      DEFAULT 0 NOT NULL,
  ROW_KIND    CHAR(1)        NOT NULL,          -- 'M'(매체), 'J'(업무), 'T'(TaskHaltStat)
  KEY_IDX1    NUMBER(2)      NOT NULL,          -- M: media_idx, J: job_idx, T: media_idx
  KEY_IDX2    NUMBER(2)      DEFAULT 0 NOT NULL,-- M/J: 0 고정, T: job_idx
  NAME_KR     VARCHAR2(64),                     -- 'M'/'J'에서만 사용(한글명)
  VAL         CHAR(1),                          -- 'T'에서만 사용(NULL/' '/'0'/'1'/기타)
  UPDATED_AT  DATE           DEFAULT SYSDATE NOT NULL


);

/* =========================================================
 * DML: 초기값 입력(매체명/TaskHaltStat)
 * - 매체명('M'): KEY_IDX1=media_idx, KEY_IDX2=0, NAME_KR=매체명, VAL=NULL
 * - 상태값('T'): KEY_IDX1=media_idx, KEY_IDX2=job_idx, VAL='1'만 저장
 *              (저장되지 않은 셀은 조회 시 0으로 출력)
 * ========================================================= */

-- 1) 매체명(M) 0..49
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, NAME_KR, VAL)
SELECT 0, 'M', media_idx, 0, media_name, NULL
FROM (
  SELECT  0 media_idx, 'IS03 M/S 신용겸용' media_name FROM dual UNION ALL
  SELECT  1, 'IS03 M/S 중앙카드' FROM dual UNION ALL
  SELECT  2, 'IS03 M/S 조합카드' FROM dual UNION ALL
  SELECT  3, 'IS03 M/S 가상계좌' FROM dual UNION ALL
  SELECT  4, 'IS03 M/S 타행카드' FROM dual UNION ALL
  SELECT  5, 'IC 자행카드' FROM dual UNION ALL
  SELECT  6, 'IC 타행카드' FROM dual UNION ALL
  SELECT  7, 'RF 자행모바일' FROM dual UNION ALL
  SELECT  8, 'RF 타행모바일' FROM dual UNION ALL
  SELECT  9, 'IRFM 모바일' FROM dual UNION ALL
  SELECT 10, '통장 요구불' FROM dual UNION ALL
  SELECT 11, '통장 신탁' FROM dual UNION ALL
  SELECT 12, '통장 저축성' FROM dual UNION ALL
  SELECT 13, '통장 수익증권' FROM dual UNION ALL
  SELECT 14, '통장 공제' FROM dual UNION ALL
  SELECT 15, '전자통장 요구불' FROM dual UNION ALL
  SELECT 16, '전자통장 신탁' FROM dual UNION ALL
  SELECT 17, '전자통장 저축성' FROM dual UNION ALL
  SELECT 18, '전자통장 수익증권' FROM dual UNION ALL
  SELECT 19, '전자화폐 자행' FROM dual UNION ALL
  SELECT 20, '전자화폐 타행' FROM dual UNION ALL
  SELECT 21, '무통장 요구불' FROM dual UNION ALL
  SELECT 22, '무통장 신탁' FROM dual UNION ALL
  SELECT 23, '무통장 저축성' FROM dual UNION ALL
  SELECT 24, '무통장 수익증권' FROM dual UNION ALL
  SELECT 25, 'IS02 M/S 자행' FROM dual UNION ALL
  SELECT 26, 'IS02 M/S 타행' FROM dual UNION ALL
  SELECT 27, 'IS02 M/S 타사' FROM dual UNION ALL
  SELECT 28, 'IS02 M/S 해외' FROM dual UNION ALL
  SELECT 29, 'IS02 EMV 자행' FROM dual UNION ALL
  SELECT 30, 'IS02 EMV 타행' FROM dual UNION ALL
  SELECT 31, 'IS02 EMV 타사' FROM dual UNION ALL
  SELECT 32, '(미정)' FROM dual UNION ALL
  SELECT 33, '(미정)' FROM dual UNION ALL
  SELECT 34, '(미정)' FROM dual UNION ALL
  SELECT 35, '(미정)' FROM dual UNION ALL
  SELECT 36, '(미정)' FROM dual UNION ALL
  SELECT 37, '(미정)' FROM dual UNION ALL
  SELECT 38, '(미정)' FROM dual UNION ALL
  SELECT 39, '(미정)' FROM dual UNION ALL
  SELECT 40, '(미정)' FROM dual UNION ALL
  SELECT 41, '(미정)' FROM dual UNION ALL
  SELECT 42, '(미정)' FROM dual UNION ALL
  SELECT 43, '(미정)' FROM dual UNION ALL
  SELECT 44, '(미정)' FROM dual UNION ALL
  SELECT 45, '(미정)' FROM dual UNION ALL
  SELECT 46, '(미정)' FROM dual UNION ALL
  SELECT 47, '(미정)' FROM dual UNION ALL
  SELECT 48, '(미정)' FROM dual UNION ALL
  SELECT 49, '(미정)' FROM dual
);

-- 2) TaskHaltStat(T): '1'인 셀만 저장
-- [05] IC 자행카드: job 3, 12, 15
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, NAME_KR, VAL)
SELECT 0, 'T', 5, job_idx, NULL, '1'
FROM (SELECT 3 job_idx FROM dual UNION ALL SELECT 12 FROM dual UNION ALL SELECT 15 FROM dual);

-- [07] RF 자행모바일: job 0, 8
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, NAME_KR, VAL)
SELECT 0, 'T', 7, job_idx, NULL, '1'
FROM (SELECT 0 job_idx FROM dual UNION ALL SELECT 8 FROM dual);

-- [10] 통장 요구불: job 7, 11, 14
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, NAME_KR, VAL)
SELECT 0, 'T', 10, job_idx, NULL, '1'
FROM (SELECT 7 job_idx FROM dual UNION ALL SELECT 11 FROM dual UNION ALL SELECT 14 FROM dual);

COMMIT;

/* =========================================================
 * SELECT: print_task_halt_stat_all() 형태(50자리 매트릭스) 출력
 *
 * 출력 정규화 규칙(main.c 로직 반영):
 * - VAL이 NULL / ' ' / '0'  -> '0' 출력
 * - VAL이 '1'               -> '1' 출력
 * - 그 외                   -> 'X' 출력
 *
 * 구현 메모:
 * - 0..49 인덱스 생성은 CONNECT BY 사용
 * - 조인은 구식 오라클 조인(+) 문법 사용
 * ========================================================= */
SELECT
  '['||LPAD(out.media_idx,2,'0')||'] '||
  RPAD(NVL(out.media_name,'(미정)'), 20, ' ')||' '||
  LISTAGG(out.out_ch,'') WITHIN GROUP (ORDER BY out.job_idx) AS line50
FROM (
  SELECT
    med.media_idx,
    job.job_idx,
    nam.NAME_KR AS media_name,
    CASE
      WHEN ths.VAL IS NULL OR ths.VAL IN (' ', '0') THEN '0'
      WHEN ths.VAL = '1' THEN '1'
      ELSE 'X'
    END AS out_ch
  FROM
    (SELECT LEVEL-1 AS media_idx FROM dual CONNECT BY LEVEL<=50) med,
    (SELECT LEVEL-1 AS job_idx  FROM dual CONNECT BY LEVEL<=50) job,
    (SELECT KEY_IDX1 AS media_idx, NAME_KR
       FROM APC_SHM_ALL
      WHERE HOST_ID=0 AND ROW_KIND='M' AND KEY_IDX2=0) nam,
    APC_SHM_ALL ths
  WHERE
    nam.media_idx(+) = med.media_idx
    AND ths.HOST_ID(+)   = 0
    AND ths.ROW_KIND(+)  = 'T'
    AND ths.KEY_IDX1(+)  = med.media_idx
    AND ths.KEY_IDX2(+)  = job.job_idx
) out
GROUP BY out.media_idx, out.media_name
ORDER BY out.media_idx;
