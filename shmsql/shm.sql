/* shm.sql
 * =========================================================
 * (Refactored) TaskHaltStat[50][50] 정규화 버전
 * =========================================================
 *
 * 목적
 * - main.c의 공유메모리 구조(Media[50], Job[50], TaskHaltStat[50][50] 등)를
 *   Oracle DB에서 "정규화된 행(row) 형태"로 재현합니다.
 *
 * 핵심 아이디어
 * - 원래 구조: TaskHaltStat[media][job] = 0/1 처럼 2차원 배열(가로/세로 매트릭스)
 * - 정규화 구조: 장애 상태 셀을 (media_idx, job_idx) 한 행으로 저장
 *
 * ROW_KIND 정의(단일 테이블에 유형을 구분해 저장)
 * - 'M' : 매체 마스터 (KEY_IDX1=매체 인덱스, NAME_KR=매체명)
 * - 'J' : 업무 마스터 (KEY_IDX1=업무 인덱스, NAME_KR=업무명)
 * - 'H' : 장애 상태  (KEY_IDX1=매체 인덱스, KEY_IDX2=업무 인덱스, VAL='1'이면 장애)
 * - 'F' : 유량제어    (KEY_IDX1=대상 인덱스, VAL='1'이면 활성)
 *
 * 주의
 * - 이 스크립트는 예제/설명용이며, 제약조건(PK/UK/CK) 및 인덱스는 최소화되어 있습니다.
 * - 다중 호스트 운용 시 HOST_ID를 조회/조인 조건에 포함하는 방식으로 확장하세요.
 */

/* =========================================================
 * DDL: 테이블 초기화 및 생성
 * ========================================================= */

-- 기존 테이블이 있다면 삭제 후 재생성할 때 사용
-- DROP TABLE APC_SHM_ALL PURGE;

CREATE TABLE APC_SHM_ALL (
  HOST_ID     NUMBER(2)      DEFAULT 0 NOT NULL, -- 호스트 ID(멀티 인스턴스 구분용)
  ROW_KIND    CHAR(1)        NOT NULL,           -- 행의 종류: 'M','J','H','F'
  KEY_IDX1    NUMBER(2)      NOT NULL,           -- 인덱스1: M/J=자기 인덱스, H=매체 인덱스, F=대상 인덱스
  KEY_IDX2    NUMBER(2)      DEFAULT 0 NOT NULL, -- 인덱스2: H에서만 업무 인덱스 사용(M/J/F는 0 권장)
  NAME_KR     VARCHAR2(64),                      -- M/J에서만 사용: 한글 명칭
  VAL         VARCHAR2(50),                      -- H/F 등 상태값(예: '1'=장애/활성)
  UPDATED_AT  DATE           DEFAULT SYSDATE NOT NULL -- 갱신 시각(기본값: 현재시각)
);

/* =========================================================
 * DML: 1) 매체 마스터 데이터 입력 ('M')
 * ========================================================= */

-- 매체 인덱스는 0~49 범위(예시). NAME_KR에 표시용 명칭을 저장합니다.
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, NAME_KR)
SELECT 0, 'M', idx, name FROM (
  SELECT  0 idx, 'IS03 M/S 신용겸용' name FROM dual UNION ALL
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

/* =========================================================
 * DML: 2) 업무 마스터 데이터 입력 ('J')
 * ========================================================= */

-- 업무 인덱스는 0~49 범위(예시). NAME_KR에 표시용 명칭을 저장합니다.
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, NAME_KR)
SELECT 0, 'J', idx, name FROM (
  SELECT  0 idx, '잔액조회' name FROM dual UNION ALL
  SELECT  1, '내역조회' FROM dual UNION ALL
  SELECT  2, '대금조회' FROM dual UNION ALL
  SELECT  3, '현금지급' FROM dual UNION ALL
  SELECT  4, '수표지급' FROM dual UNION ALL
  SELECT  5, '현금입금' FROM dual UNION ALL
  SELECT  6, '수표입금' FROM dual UNION ALL
  SELECT  7, 'CMS집금' FROM dual UNION ALL
  SELECT  8, '계좌이체' FROM dual UNION ALL
  SELECT  9, 'CMS이체' FROM dual UNION ALL
  SELECT 10, '카드론이체' FROM dual UNION ALL
  SELECT 11, '입금이체' FROM dual UNION ALL
  SELECT 12, '출금이체' FROM dual UNION ALL
  SELECT 13, '동행이체' FROM dual UNION ALL
  SELECT 14, '삼행이체' FROM dual UNION ALL
  SELECT 15, '통장정리' FROM dual UNION ALL
  SELECT 16, '아파트관리비' FROM dual UNION ALL
  SELECT 17, '공과금수납' FROM dual UNION ALL
  SELECT 18, '사고신고' FROM dual UNION ALL
  SELECT 19, '비밀번호변경' FROM dual UNION ALL
  SELECT 20, '전자화폐충전환불' FROM dual UNION ALL
  SELECT 21, '신용선결제' FROM dual UNION ALL
  SELECT 22, '신용청구대금결제' FROM dual UNION ALL
  SELECT 23, '대학등록금' FROM dual UNION ALL
  SELECT 24, '공제납부' FROM dual UNION ALL
  SELECT 25, '여신상환' FROM dual UNION ALL
  SELECT 26, '지방세' FROM dual UNION ALL
  SELECT 27, '전자납부' FROM dual UNION ALL
  SELECT 28, '2D바코드' FROM dual UNION ALL
  SELECT 29, '(미정)' FROM dual UNION ALL
  SELECT 30, '(미정)' FROM dual UNION ALL
  SELECT 31, '(미정)' FROM dual UNION ALL
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

/* =========================================================
 * DML: 3) 장애 상태 데이터 입력 ('H' - Normalized)
 * ========================================================= */

-- 장애 상태는 "장애인 셀만" 행으로 저장합니다(정규화).
-- 즉 (매체 KEY_IDX1, 업무 KEY_IDX2, VAL='1') 조합이 존재하면 장애로 해석합니다.
-- 장애가 아닌 셀은 행이 없으므로, 조회 시 LEFT JOIN + NVL로 '0'을 채웁니다.

-- [05] IC 자행카드: 3(현금지급), 12(출금이체), 15(통장정리) 장애
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, VAL) VALUES (0, 'H', 5, 3, '1');
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, VAL) VALUES (0, 'H', 5, 12, '1');
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, VAL) VALUES (0, 'H', 5, 15, '1');

-- [07] RF 자행모바일: 0(잔액조회), 8(계좌이체) 장애
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, VAL) VALUES (0, 'H', 7, 0, '1');
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, VAL) VALUES (0, 'H', 7, 8, '1');

-- [10] 통장 요구불: 7(CMS집금), 11(입금이체), 14(삼행이체) 장애
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, VAL) VALUES (0, 'H', 10, 7, '1');
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, VAL) VALUES (0, 'H', 10, 11, '1');
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, KEY_IDX2, VAL) VALUES (0, 'H', 10, 14, '1');

/* =========================================================
 * DML: 4) 유량제어 데이터 입력 ('F')
 * =========================================================
 *
 * 의미(예시)
 * - KEY_IDX1에 대상 인덱스를 넣고, VAL='1'이면 "유량제어 활성"로 간주합니다.
 * - 실제 의미(대상이 매체인지/업무인지/별도 코드인지)는 운영 규칙에 맞게 확정하세요.
 */

-- 예시: 2번, 5번 유량제어 활성
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, VAL) VALUES (0, 'F', 2, '1');
INSERT INTO APC_SHM_ALL (HOST_ID, ROW_KIND, KEY_IDX1, VAL) VALUES (0, 'F', 5, '1');

-- 여기까지가 샘플 데이터 적재 파트입니다.
COMMIT;

/* =========================================================
 * SELECT: 유량제어 대상 조회
 * ========================================================= */

-- 활성(VAL='1')인 유량제어 대상을 나열합니다.
SELECT
  '['||KEY_IDX1||']번 유량제어 활성' AS flow_control_status
FROM
  APC_SHM_ALL
WHERE
  ROW_KIND = 'F'
  AND VAL = '1'
ORDER BY KEY_IDX1;

/* =========================================================
 * SELECT: 매체별 50자리 장애 상태 문자열 조회 (정규화 데이터 -> 가로 출력)
 * ========================================================= */

-- 목적
-- - TaskHaltStat[media][job] 형태의 0/1 문자열(길이 50)을 매체별로 재구성합니다.
-- 원리
-- - pos(0~49)를 생성한 뒤, H(장애) 행과 LEFT JOIN
-- - 장애 행이 있으면 '1', 없으면 NVL로 '0'
-- - ORDER BY pos.lvl 순서대로 LISTAGG로 이어 붙여 50자리 문자열 생성
SELECT
  '['||LPAD(med.KEY_IDX1, 2, '0')||'] '||
  RPAD(med.NAME_KR, 20, ' ')||' '||
  (
    SELECT LISTAGG(NVL(hlt.VAL, '0'), '') WITHIN GROUP (ORDER BY pos.lvl)
    FROM (SELECT LEVEL - 1 AS lvl FROM DUAL CONNECT BY LEVEL <= 50) pos
    LEFT JOIN APC_SHM_ALL hlt ON hlt.ROW_KIND = 'H'
                             AND hlt.KEY_IDX1 = med.KEY_IDX1
                             AND hlt.KEY_IDX2 = pos.lvl
  ) AS line50
FROM
  APC_SHM_ALL med
WHERE
  med.ROW_KIND = 'M'
ORDER BY med.KEY_IDX1;

/* =========================================================
 * SELECT: 인간이 읽기 좋은 장애 요약 조회 (명칭 기반)
 * ========================================================= */

-- 목적
-- - 장애 셀(H)을 사람이 읽기 쉬운 "매체명 + 업무명" 형태로 요약 출력합니다.
-- 주의
-- - 현재는 HOST_ID를 조건에 포함하지 않습니다(단일 호스트 가정).
SELECT
  '['||LPAD(med.KEY_IDX1, 2, '0')||':'||RPAD(med.NAME_KR, 20, ' ')||'] '||
  '<'||LPAD(job.KEY_IDX1, 2, '0')||':'||RPAD(job.NAME_KR, 20, ' ')||'>' AS halt_summary
FROM
  APC_SHM_ALL med, -- 매체 마스터
  APC_SHM_ALL job, -- 업무 마스터
  APC_SHM_ALL hlt  -- 장애 상태
WHERE
  med.ROW_KIND = 'M'
  AND job.ROW_KIND = 'J'
  AND hlt.ROW_KIND = 'H'
  AND med.KEY_IDX1 = hlt.KEY_IDX1
  AND job.KEY_IDX1 = hlt.KEY_IDX2
ORDER BY med.KEY_IDX1, job.KEY_IDX1;
